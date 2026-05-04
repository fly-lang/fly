#include "Checksum.h"

#include <array>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

// Self-contained SHA-256 implementation — no external dependencies.
namespace flyp::detail {

static constexpr uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t Ch(uint32_t x, uint32_t y, uint32_t z)  { return (x & y) ^ (~x & z); }
static inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t Sigma0(uint32_t x) { return rotr(x,2)  ^ rotr(x,13) ^ rotr(x,22); }
static inline uint32_t Sigma1(uint32_t x) { return rotr(x,6)  ^ rotr(x,11) ^ rotr(x,25); }
static inline uint32_t sigma0(uint32_t x) { return rotr(x,7)  ^ rotr(x,18) ^ (x >> 3);   }
static inline uint32_t sigma1(uint32_t x) { return rotr(x,17) ^ rotr(x,19) ^ (x >> 10);  }

struct SHA256Ctx {
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
    };
    uint64_t bit_count = 0;
    uint8_t  buf[64]   = {};
    size_t   buf_len   = 0;

    void process_block(const uint8_t* blk) {
        uint32_t W[64];
        for (int i = 0; i < 16; ++i)
            W[i] = (uint32_t(blk[i*4])<<24) | (uint32_t(blk[i*4+1])<<16)
                 | (uint32_t(blk[i*4+2])<<8) | uint32_t(blk[i*4+3]);
        for (int i = 16; i < 64; ++i)
            W[i] = sigma1(W[i-2]) + W[i-7] + sigma0(W[i-15]) + W[i-16];

        uint32_t a=state[0], b=state[1], c=state[2], d=state[3],
                 e=state[4], f=state[5], g=state[6], h=state[7];
        for (int i = 0; i < 64; ++i) {
            uint32_t T1 = h + Sigma1(e) + Ch(e,f,g) + K[i] + W[i];
            uint32_t T2 = Sigma0(a) + Maj(a,b,c);
            h=g; g=f; f=e; e=d+T1; d=c; c=b; b=a; a=T1+T2;
        }
        state[0]+=a; state[1]+=b; state[2]+=c; state[3]+=d;
        state[4]+=e; state[5]+=f; state[6]+=g; state[7]+=h;
    }

    void update(const uint8_t* data, size_t len) {
        bit_count += len * 8;
        while (len > 0) {
            size_t take = std::min(len, 64 - buf_len);
            std::memcpy(buf + buf_len, data, take);
            buf_len += take;
            data    += take;
            len     -= take;
            if (buf_len == 64) { process_block(buf); buf_len = 0; }
        }
    }

    std::array<uint8_t,32> finalize() {
        buf[buf_len++] = 0x80;
        if (buf_len > 56) {
            while (buf_len < 64) buf[buf_len++] = 0;
            process_block(buf); buf_len = 0;
        }
        while (buf_len < 56) buf[buf_len++] = 0;
        for (int i = 7; i >= 0; --i) buf[buf_len++] = uint8_t(bit_count >> (i*8));
        process_block(buf);

        std::array<uint8_t,32> digest{};
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 4; ++j)
                digest[i*4+j] = uint8_t(state[i] >> (24 - j*8));
        return digest;
    }
};

static std::string to_hex(const std::array<uint8_t,32>& d) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (auto b : d) ss << std::setw(2) << int(b);
    return ss.str();
}

} // namespace flyp::detail

namespace flyp {

std::string sha256_string(const std::string& data) {
    detail::SHA256Ctx ctx;
    ctx.update(reinterpret_cast<const uint8_t*>(data.data()), data.size());
    return "sha256:" + detail::to_hex(ctx.finalize());
}

std::string sha256_file(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open file: " + path.string());

    detail::SHA256Ctx ctx;
    std::array<uint8_t, 4096> buf{};
    while (f.read(reinterpret_cast<char*>(buf.data()), buf.size()) || f.gcount())
        ctx.update(buf.data(), static_cast<size_t>(f.gcount()));

    return "sha256:" + detail::to_hex(ctx.finalize());
}

bool verify_checksum(const std::filesystem::path& path, const std::string& expected) {
    return sha256_file(path) == expected;
}

} // namespace flyp
