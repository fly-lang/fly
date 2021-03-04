//===--- Mips.cpp - Implement Mips target feature support -----------------===//
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// This file implements Mips TargetInfo objects.
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Basic/Targets/Mips.h"
#include "Basic/Targets/Targets.h"
#include "Basic/Diagnostic.h"
#include "Basic/TargetBuiltins.h"
#include "llvm/ADT/StringSwitch.h"

using namespace fly;
using namespace fly::targets;

const Builtin::Info MipsTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER)                                    \
  {#ID, TYPE, ATTRS, HEADER, ALL_LANGUAGES, nullptr},

#include "Basic/BuiltinsMips.def"
};

bool MipsTargetInfo::processorSupportsGPR64() const {
    return llvm::StringSwitch<bool>(CPU)
            .Case("mips3", true)
            .Case("mips4", true)
            .Case("mips5", true)
            .Case("mips64", true)
            .Case("mips64r2", true)
            .Case("mips64r3", true)
            .Case("mips64r5", true)
            .Case("mips64r6", true)
            .Case("octeon", true)
            .Case("octeon+", true)
            .Default(false);
    return false;
}

static constexpr llvm::StringLiteral ValidCPUNames[] = {
        {"mips1"},
        {"mips2"},
        {"mips3"},
        {"mips4"},
        {"mips5"},
        {"mips32"},
        {"mips32r2"},
        {"mips32r3"},
        {"mips32r5"},
        {"mips32r6"},
        {"mips64"},
        {"mips64r2"},
        {"mips64r3"},
        {"mips64r5"},
        {"mips64r6"},
        {"octeon"},
        {"octeon+"},
        {"p5600"}};

bool MipsTargetInfo::isValidCPUName(StringRef Name) const {
    return llvm::find(ValidCPUNames, Name) != std::end(ValidCPUNames);
}

void MipsTargetInfo::fillValidCPUList(
        SmallVectorImpl<StringRef> &Values) const {
    Values.append(std::begin(ValidCPUNames), std::end(ValidCPUNames));
}

unsigned MipsTargetInfo::getISARev() const {
    return llvm::StringSwitch<unsigned>(getCPU())
            .Cases("mips32", "mips64", 1)
            .Cases("mips32r2", "mips64r2", "octeon", "octeon+", 2)
            .Cases("mips32r3", "mips64r3", 3)
            .Cases("mips32r5", "mips64r5", 5)
            .Cases("mips32r6", "mips64r6", 6)
            .Default(0);
}

bool MipsTargetInfo::hasFeature(StringRef Feature) const {
    return llvm::StringSwitch<bool>(Feature)
            .Case("mips", true)
            .Case("dsp", DspRev >= DSP1)
            .Case("dspr2", DspRev >= DSP2)
            .Case("fp64", FPMode == FP64)
            .Case("msa", HasMSA)
            .Default(false);
}

ArrayRef<Builtin::Info> MipsTargetInfo::getTargetBuiltins() const {
    return llvm::makeArrayRef(BuiltinInfo, fly::Mips::LastTSBuiltin -
                                           Builtin::FirstTSBuiltin);
}

unsigned MipsTargetInfo::getUnwindWordWidth() const {
    return llvm::StringSwitch<unsigned>(ABI)
            .Case("o32", 32)
            .Case("n32", 64)
            .Case("n64", 64)
            .Default(getPointerWidth(0));
}

bool MipsTargetInfo::validateTarget(DiagnosticsEngine &Diags) const {
    // microMIPS64R6 backend was removed.
    if (getTriple().isMIPS64() && IsMicromips && (ABI == "n32" || ABI == "n64")) {
        Diags.Report(diag::err_target_unsupported_cpu_for_micromips) << CPU;
        return false;
    }
    // FIXME: It's valid to use O32 on a 64-bit CPU but the backend can't handle
    //        this yet. It's better to fail here than on the backend assertion.
    if (processorSupportsGPR64() && ABI == "o32") {
        Diags.Report(diag::err_target_unsupported_abi) << ABI << CPU;
        return false;
    }

    // 64-bit ABI's require 64-bit CPU's.
    if (!processorSupportsGPR64() && (ABI == "n32" || ABI == "n64")) {
        Diags.Report(diag::err_target_unsupported_abi) << ABI << CPU;
        return false;
    }

    // FIXME: It's valid to use O32 on a mips64/mips64el triple but the backend
    //        can't handle this yet. It's better to fail here than on the
    //        backend assertion.
    if (getTriple().isMIPS64() && ABI == "o32") {
        Diags.Report(diag::err_target_unsupported_abi_for_triple)
                << ABI << getTriple().str();
        return false;
    }

    // FIXME: It's valid to use N32/N64 on a mips/mipsel triple but the backend
    //        can't handle this yet. It's better to fail here than on the
    //        backend assertion.
    if (getTriple().isMIPS32() && (ABI == "n32" || ABI == "n64")) {
        Diags.Report(diag::err_target_unsupported_abi_for_triple)
                << ABI << getTriple().str();
        return false;
    }

    // -fpxx is valid only for the o32 ABI
    if (FPMode == FPXX && (ABI == "n32" || ABI == "n64")) {
        Diags.Report(diag::err_unsupported_abi_for_opt) << "-mfpxx" << "o32";
        return false;
    }

    // -mfp32 and n32/n64 ABIs are incompatible
    if (FPMode != FP64 && FPMode != FPXX && !IsSingleFloat &&
        (ABI == "n32" || ABI == "n64")) {
        Diags.Report(diag::err_opt_not_valid_with_opt) << "-mfpxx" << CPU;
        return false;
    }
    // Mips revision 6 and -mfp32 are incompatible
    if (FPMode != FP64 && FPMode != FPXX && (CPU == "mips32r6" ||
                                             CPU == "mips64r6")) {
        Diags.Report(diag::err_opt_not_valid_with_opt) << "-mfp32" << CPU;
        return false;
    }
    // Option -mfp64 permitted on Mips32 iff revision 2 or higher is present
    if (FPMode == FP64 && (CPU == "mips1" || CPU == "mips2" ||
                           getISARev() < 2) && ABI == "o32") {
        Diags.Report(diag::err_mips_fp64_req) << "-mfp64";
        return false;
    }

    return true;
}
