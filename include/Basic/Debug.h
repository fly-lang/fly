//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/Debug.cpp - Debug
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_BASIC_DEBUG_H
#define FLY_BASIC_DEBUG_H

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <chrono>

#define DEBUG_TYPE "FLY_DEBUG"

extern bool DebugLog;
extern thread_local int DebugDepth;

#define FLY_DEBUG_WITH_TYPE(TYPE, X) \
  do { if (DebugLog) { X; } \
  } while (false)

// Print one debug line.
// Format: [Debug] (│ )*depth  prefix  label\n
// dir: '>' = scope entry  → ┌─
//      '<' = scope exit   → └─
//      '*' = standalone   → ·
inline void DebugPrintLine(char dir, llvm::StringRef label) noexcept {
    auto &OS = llvm::dbgs();
    OS << "[Debug] ";
    for (int i = 0; i < DebugDepth; ++i) OS << "\xe2\x94\x82 "; // │
    if      (dir == '>') OS << "\xe2\x94\x8c\xe2\x94\x80 ";     // ┌─
    else if (dir == '<') OS << "\xe2\x94\x94\xe2\x94\x80 ";     // └─
    else                 OS << "\xc2\xb7 ";                      // ·
    OS << label << '\n';
}

// RAII scope guard.
// '>' on construction (with optional context message), '<' + timing on destruction.
// Guarantees matched pairs even on early return. No heap, no RTTI, no exceptions.
struct DebugScope {
    const char *Class;
    const char *Method;
    std::chrono::steady_clock::time_point T0;

    DebugScope(const char *cls, const char *method,
               llvm::StringRef msg = {}) noexcept
        : Class(cls), Method(method) {
        if (!DebugLog) return;
        T0 = std::chrono::steady_clock::now();
        llvm::SmallString<128> label;
        {
            llvm::raw_svector_ostream lo(label);
            lo << cls << "::" << method;
            if (!msg.empty()) lo << " | " << msg;
        }
        DebugPrintLine('>', llvm::StringRef(label));
        ++DebugDepth;
    }

    ~DebugScope() noexcept {
        if (!DebugLog) return;
        --DebugDepth;
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - T0).count();
        llvm::SmallString<64> label;
        {
            llvm::raw_svector_ostream lo(label);
            lo << Class << "::" << Method;
            if (us >= 1000) lo << " (" << (us / 1000) << "ms)";
            else            lo << " (" << us << "us)";
        }
        DebugPrintLine('<', llvm::StringRef(label));
    }
};

// Replaces paired FLY_DEBUG_START + FLY_DEBUG_END
#define FLY_DEBUG_SCOPE(CLASS, METHOD) \
    DebugScope _dbg_scope_(CLASS, METHOD)

// Replaces paired FLY_DEBUG_START_MSG + FLY_DEBUG_END.
// MSG is an ostream expression (e.g. "name=" << x).
// Built into a stack-allocated SmallString — no heap, no exceptions.
#define FLY_DEBUG_SCOPE_MSG(CLASS, METHOD, MSG)                          \
    llvm::SmallString<128> _dbg_msg_;                                    \
    do { if (DebugLog) {                                             \
        llvm::raw_svector_ostream _dbg_o_(_dbg_msg_); _dbg_o_ << MSG;   \
    } } while (false);                                                   \
    DebugScope _dbg_scope_(CLASS, METHOD, llvm::StringRef(_dbg_msg_))

// One-liner event with no matching end (e.g. "Set -debug", return-value trace).
// Uses '*' as direction char.
#define FLY_DEBUG_MSG(MSG)                                               \
    do { if (DebugLog) {                                             \
        llvm::SmallString<128> _m_;                                      \
        { llvm::raw_svector_ostream _o_(_m_); _o_ << MSG; }             \
        DebugPrintLine('*', llvm::StringRef(_m_));                       \
    } } while (false)

#endif
