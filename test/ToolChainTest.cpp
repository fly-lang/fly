//===--------------------------------------------------------------------------------------------------------------===//
// test/ToolChainTest.cpp - ToolChain unit tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "TestUtils.h"
#include "Basic/Archiver.h"
#include "Driver/ToolChain.h"
#include "Basic/CodeGenOptions.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/TargetParser/Triple.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>

namespace {
    using namespace fly;
    using ArchType = llvm::Triple::ArchType;

    // ─── Fixture ─────────────────────────────────────────────────────────────

    class ToolChainTest : public ::testing::Test {
    protected:
        std::shared_ptr<CompilerInstance> CI;
        CodeGenOptions CGOpts;

        ToolChainTest() : CI(TestUtils::CreateCompilerInstance()) {}

        ToolChain makeTC(llvm::StringRef TripleStr) {
            T = llvm::Triple(llvm::Triple::normalize(TripleStr));
            return ToolChain(CI->getDiagnostics(), T, CGOpts);
        }

        llvm::Triple T;
    };

    // ─── getPIE ──────────────────────────────────────────────────────────────

    TEST_F(ToolChainTest, GetPIE_DefaultStaticIsFalse) {
        // Default: Static=true, PIE=false → getPIE() returns false
        ToolChain TC = makeTC("x86_64-linux-gnu");
        EXPECT_FALSE(TC.getPIE());
    }

    TEST_F(ToolChainTest, GetPIE_SharedDisablesPIE) {
        CGOpts.Static = false;
        CGOpts.Shared = true;
        CGOpts.PIE   = true;
        ToolChain TC = makeTC("x86_64-linux-gnu");
        EXPECT_FALSE(TC.getPIE());
    }

    TEST_F(ToolChainTest, GetPIE_StaticDisablesPIE) {
        CGOpts.Static = true;
        CGOpts.Shared = false;
        CGOpts.PIE   = true;
        ToolChain TC = makeTC("x86_64-linux-gnu");
        EXPECT_FALSE(TC.getPIE());
    }

    TEST_F(ToolChainTest, GetPIE_TrueWhenOnlyPIESet) {
        CGOpts.Static = false;
        CGOpts.Shared = false;
        CGOpts.PIE   = true;
        ToolChain TC = makeTC("x86_64-linux-gnu");
        EXPECT_TRUE(TC.getPIE());
    }

    TEST_F(ToolChainTest, GetPIE_FalseWhenAllClear) {
        CGOpts.Static = false;
        CGOpts.Shared = false;
        CGOpts.PIE   = false;
        ToolChain TC = makeTC("x86_64-linux-gnu");
        EXPECT_FALSE(TC.getPIE());
    }

    // ─── isArmBigEndian ──────────────────────────────────────────────────────

    TEST_F(ToolChainTest, IsArmBigEndian_Armeb) {
        ToolChain TC = makeTC("armeb-linux-gnueabi");
        EXPECT_TRUE(TC.isArmBigEndian());
    }

    TEST_F(ToolChainTest, IsArmBigEndian_Thumbeb) {
        ToolChain TC = makeTC("thumbeb-linux-gnueabi");
        EXPECT_TRUE(TC.isArmBigEndian());
    }

    TEST_F(ToolChainTest, IsArmBigEndian_ArmLittleEndian) {
        ToolChain TC = makeTC("arm-linux-gnueabi");
        EXPECT_FALSE(TC.isArmBigEndian());
    }

    TEST_F(ToolChainTest, IsArmBigEndian_Thumb) {
        ToolChain TC = makeTC("thumb-linux-gnueabi");
        EXPECT_FALSE(TC.isArmBigEndian());
    }

    TEST_F(ToolChainTest, IsArmBigEndian_X86_64) {
        ToolChain TC = makeTC("x86_64-linux-gnu");
        EXPECT_FALSE(TC.isArmBigEndian());
    }

    TEST_F(ToolChainTest, IsArmBigEndian_Aarch64) {
        ToolChain TC = makeTC("aarch64-linux-gnu");
        EXPECT_FALSE(TC.isArmBigEndian());
    }

    // ─── getLDMOption ────────────────────────────────────────────────────────

    TEST_F(ToolChainTest, GetLDMOption_X86_64) {
        ToolChain TC = makeTC("x86_64-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf_x86_64");
    }

    TEST_F(ToolChainTest, GetLDMOption_X86) {
        ToolChain TC = makeTC("i386-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf_i386");
    }

    TEST_F(ToolChainTest, GetLDMOption_Aarch64) {
        ToolChain TC = makeTC("aarch64-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "aarch64linux");
    }

    TEST_F(ToolChainTest, GetLDMOption_Aarch64BE) {
        ToolChain TC = makeTC("aarch64_be-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "aarch64linuxb");
    }

    TEST_F(ToolChainTest, GetLDMOption_ArmLE) {
        ToolChain TC = makeTC("arm-linux-gnueabi");
        EXPECT_STREQ(TC.getLDMOption(), "armelf_linux_eabi");
    }

    TEST_F(ToolChainTest, GetLDMOption_ArmBE) {
        ToolChain TC = makeTC("armeb-linux-gnueabi");
        EXPECT_STREQ(TC.getLDMOption(), "armelfb_linux_eabi");
    }

    TEST_F(ToolChainTest, GetLDMOption_Thumb) {
        ToolChain TC = makeTC("thumb-linux-gnueabi");
        EXPECT_STREQ(TC.getLDMOption(), "armelf_linux_eabi");
    }

    TEST_F(ToolChainTest, GetLDMOption_PPC) {
        ToolChain TC = makeTC("powerpc-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf32ppclinux");
    }

    TEST_F(ToolChainTest, GetLDMOption_PPC64) {
        ToolChain TC = makeTC("powerpc64-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf64ppc");
    }

    TEST_F(ToolChainTest, GetLDMOption_PPC64LE) {
        ToolChain TC = makeTC("powerpc64le-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf64lppc");
    }

    TEST_F(ToolChainTest, GetLDMOption_RISCV32) {
        ToolChain TC = makeTC("riscv32-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf32lriscv");
    }

    TEST_F(ToolChainTest, GetLDMOption_RISCV64) {
        ToolChain TC = makeTC("riscv64-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf64lriscv");
    }

    TEST_F(ToolChainTest, GetLDMOption_Sparc) {
        ToolChain TC = makeTC("sparc-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf32_sparc");
    }

    TEST_F(ToolChainTest, GetLDMOption_SparcV9) {
        ToolChain TC = makeTC("sparcv9-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf64_sparc");
    }

    TEST_F(ToolChainTest, GetLDMOption_Mips) {
        ToolChain TC = makeTC("mips-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf32btsmip");
    }

    TEST_F(ToolChainTest, GetLDMOption_MipsEL) {
        ToolChain TC = makeTC("mipsel-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf32ltsmip");
    }

    TEST_F(ToolChainTest, GetLDMOption_Mips64) {
        ToolChain TC = makeTC("mips64-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf64btsmip");
    }

    TEST_F(ToolChainTest, GetLDMOption_Mips64EL) {
        ToolChain TC = makeTC("mips64el-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf64ltsmip");
    }

    TEST_F(ToolChainTest, GetLDMOption_SystemZ) {
        ToolChain TC = makeTC("s390x-linux-gnu");
        EXPECT_STREQ(TC.getLDMOption(), "elf64_s390");
    }

    TEST_F(ToolChainTest, GetLDMOption_NullForUnknown) {
        // wasm32 is not in the switch → nullptr
        ToolChain TC = makeTC("wasm32-unknown-unknown");
        EXPECT_EQ(TC.getLDMOption(), nullptr);
    }

    // ─── getOSLibDir ─────────────────────────────────────────────────────────

    TEST_F(ToolChainTest, GetOSLibDir_X86_64) {
        ToolChain TC = makeTC("x86_64-linux-gnu");
        EXPECT_EQ(TC.getOSLibDir(), "lib64");
    }

    TEST_F(ToolChainTest, GetOSLibDir_X86) {
        ToolChain TC = makeTC("i386-linux-gnu");
        EXPECT_EQ(TC.getOSLibDir(), "lib32");
    }

    TEST_F(ToolChainTest, GetOSLibDir_PPC) {
        ToolChain TC = makeTC("powerpc-linux-gnu");
        EXPECT_EQ(TC.getOSLibDir(), "lib32");
    }

    TEST_F(ToolChainTest, GetOSLibDir_PPC64) {
        ToolChain TC = makeTC("powerpc64-linux-gnu");
        EXPECT_EQ(TC.getOSLibDir(), "lib64");
    }

    TEST_F(ToolChainTest, GetOSLibDir_Aarch64) {
        ToolChain TC = makeTC("aarch64-linux-gnu");
        EXPECT_EQ(TC.getOSLibDir(), "lib64");
    }

    TEST_F(ToolChainTest, GetOSLibDir_RISCV32) {
        ToolChain TC = makeTC("riscv32-linux-gnu");
        EXPECT_EQ(TC.getOSLibDir(), "lib32");
    }

    TEST_F(ToolChainTest, GetOSLibDir_RISCV64) {
        ToolChain TC = makeTC("riscv64-linux-gnu");
        EXPECT_EQ(TC.getOSLibDir(), "lib64");
    }

    TEST_F(ToolChainTest, GetOSLibDir_Arm) {
        ToolChain TC = makeTC("arm-linux-gnueabi");
        EXPECT_EQ(TC.getOSLibDir(), "lib");
    }

    // ─── getMultiarch (Android - no VFS probe) ───────────────────────────────

    TEST_F(ToolChainTest, GetMultiarch_AndroidArm) {
        ToolChain TC = makeTC("arm-linux-androideabi");
        EXPECT_EQ(TC.getMultiarch(), "arm-linux-androideabi");
    }

    TEST_F(ToolChainTest, GetMultiarch_AndroidAarch64) {
        ToolChain TC = makeTC("aarch64-linux-android");
        EXPECT_EQ(TC.getMultiarch(), "aarch64-linux-android");
    }

    TEST_F(ToolChainTest, GetMultiarch_AndroidX86) {
        ToolChain TC = makeTC("i686-linux-android");
        EXPECT_EQ(TC.getMultiarch(), "i686-linux-android");
    }

    TEST_F(ToolChainTest, GetMultiarch_AndroidX86_64) {
        ToolChain TC = makeTC("x86_64-linux-android");
        EXPECT_EQ(TC.getMultiarch(), "x86_64-linux-android");
    }

    TEST_F(ToolChainTest, GetMultiarch_AndroidMipsel) {
        ToolChain TC = makeTC("mipsel-linux-android");
        EXPECT_EQ(TC.getMultiarch(), "mipsel-linux-android");
    }

    TEST_F(ToolChainTest, GetMultiarch_AndroidMips64el) {
        ToolChain TC = makeTC("mips64el-linux-android");
        EXPECT_EQ(TC.getMultiarch(), "mips64el-linux-android");
    }

    // ─── getVFS ──────────────────────────────────────────────────────────────

    TEST_F(ToolChainTest, GetVFS_NotNull) {
        ToolChain TC = makeTC("x86_64-linux-gnu");
        // Just verify the VFS is accessible and non-null by calling exists on "/"
        EXPECT_TRUE(TC.getVFS().exists("/"));
    }

    // ─── GetFilePath ─────────────────────────────────────────────────────────

    TEST_F(ToolChainTest, GetFilePath_ReturnsEmptyWhenNotFound) {
        ToolChain TC = makeTC("x86_64-linux-gnu");
        llvm::SmallVector<std::string, 4> Paths = {"/tmp"};
        llvm::SmallVector<std::string, 16> PathList(Paths.begin(), Paths.end());
        // A file that surely does not exist
        std::string result = TC.GetFilePath("fly_nonexistent_xyz.o", PathList);
        EXPECT_TRUE(result.empty());
    }

    TEST_F(ToolChainTest, GetFilePath_FindsExistingFile) {
        ToolChain TC = makeTC("x86_64-linux-gnu");

        auto tmpDir  = std::filesystem::temp_directory_path();
        std::string tmpPath = (tmpDir / "fly_getfilepath_test.o").string();
        { std::ofstream f(tmpPath); f << ""; }

        llvm::SmallVector<std::string, 16> PathList = {tmpDir.string()};
        std::string result = TC.GetFilePath("fly_getfilepath_test.o", PathList);
        EXPECT_EQ(result, tmpPath);

        remove(tmpPath.c_str());
    }

    TEST_F(ToolChainTest, GetFilePath_SkipsEmptyPaths) {
        ToolChain TC = makeTC("x86_64-linux-gnu");
        llvm::SmallVector<std::string, 16> PathList = {"", "/tmp"};
        // nonexistent → empty result even with valid dir
        std::string result = TC.GetFilePath("fly_nonexistent_xyz.o", PathList);
        EXPECT_TRUE(result.empty());
    }

    // ─── Archiver::CreateLib (defined in ToolChain.cpp) ──────────────────────

    TEST_F(ToolChainTest, CreateLib_CreatesArchive) {
        auto tmpDir = std::filesystem::temp_directory_path();
        std::string f1  = (tmpDir / "fly_clib_1.o").string();
        std::string f2  = (tmpDir / "fly_clib_2.o").string();
        std::string arc = (tmpDir / "fly_clib_test.a").string();

        { std::ofstream f(f1); f << "obj1"; }
        { std::ofstream f(f2); f << "obj2"; }
        remove(arc.c_str());

        Archiver ar(CI->getDiagnostics(), arc);
        llvm::SmallVector<std::string, 4> files = {f1, f2};
        bool ok = ar.CreateLib(files);
        EXPECT_TRUE(ok);
        EXPECT_TRUE(llvm::sys::fs::exists(arc));

        remove(f1.c_str()); remove(f2.c_str()); remove(arc.c_str());
    }

    TEST_F(ToolChainTest, CreateLib_OverrideIfArchiveAlreadyExists) {
        std::string arc = (std::filesystem::temp_directory_path() / "fly_clib_exists.a").string();

        { std::ofstream f(arc); f << "already here"; }

        Archiver ar(CI->getDiagnostics(), arc);
        llvm::SmallVector<std::string, 4> files;
        bool ok = ar.CreateLib(files);
        EXPECT_TRUE(ok);

        remove(arc.c_str());
    }

    // ─── GetRuntimeLibPath ───────────────────────────────────────────────────

#ifndef _WIN32
    TEST_F(ToolChainTest, GetCompilerRTBuiltinsPath_ReturnsNonEmptyOnLinuxX86_64) {
        ToolChain TC = makeTC("x86_64-linux-gnu");
        std::string P = TC.GetCompilerRTBuiltinsPath();
        EXPECT_FALSE(P.empty()) << "libclang_rt.builtins-x86_64.a not found";
        EXPECT_TRUE(llvm::sys::fs::exists(P)) << "Path returned but file missing: " << P;
    }
#endif

    TEST_F(ToolChainTest, GetCompilerRTBuiltinsPath_EmptyForUnknownArch) {
        ToolChain TC = makeTC("wasm32-unknown-unknown");
        EXPECT_TRUE(TC.GetCompilerRTBuiltinsPath().empty());
    }

#ifndef _WIN32
    TEST_F(ToolChainTest, GetRuntimeLibPath_ReturnsNonEmptyOnThisBuild) {
        CGOpts.RuntimeLibDir = FLY_TEST_LIB_DIR;
        ToolChain TC = makeTC("x86_64-linux-gnu");
        std::string P = TC.GetRuntimeLibPath();
        EXPECT_FALSE(P.empty()) << "fly_runtime_lib.a not found in " FLY_TEST_LIB_DIR;
        EXPECT_TRUE(llvm::sys::fs::exists(P)) << "Path returned but file missing: " << P;
    }
#endif

    TEST_F(ToolChainTest, CreateLib_EmptyFileList) {
        std::string arc = (std::filesystem::temp_directory_path() / "fly_clib_empty.a").string();
        remove(arc.c_str());

        Archiver ar(CI->getDiagnostics(), arc);
        llvm::SmallVector<std::string, 4> files;
        bool ok = ar.CreateLib(files);
        EXPECT_TRUE(ok);
        EXPECT_TRUE(llvm::sys::fs::exists(arc));

        remove(arc.c_str());
    }

} // anonymous namespace
