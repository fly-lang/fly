//===--- WebAssembly.cpp - Implement WebAssembly target feature support ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements WebAssembly TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "WebAssembly.h"
#include "Basic/Targets.h"
#include "Basic/Builtins.h"
#include "Basic/Diagnostic.h"
#include "Basic/TargetBuiltins.h"
#include "llvm/ADT/StringSwitch.h"

using namespace fly;
using namespace fly::targets;

const Builtin::Info WebAssemblyTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE)                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, FEATURE},
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER)                                    \
  {#ID, TYPE, ATTRS, HEADER, ALL_LANGUAGES, nullptr},
#include "Basic/BuiltinsWebAssembly.def"
};

static constexpr llvm::StringLiteral ValidCPUNames[] = {
    {"mvp"}, {"bleeding-edge"}, {"generic"}};

StringRef WebAssemblyTargetInfo::getABI() const { return ABI; }

bool WebAssemblyTargetInfo::setABI(const std::string &Name) {
  if (Name != "mvp" && Name != "experimental-mv")
    return false;

  ABI = Name;
  return true;
}

bool WebAssemblyTargetInfo::hasFeature(StringRef Feature) const {
  return llvm::StringSwitch<bool>(Feature)
      .Case("simd128", SIMDLevel >= SIMD128)
      .Case("unimplemented-simd128", SIMDLevel >= UnimplementedSIMD128)
      .Case("nontrapping-fptoint", HasNontrappingFPToInt)
      .Case("sign-ext", HasSignExt)
      .Case("exception-handling", HasExceptionHandling)
      .Case("bulk-memory", HasBulkMemory)
      .Case("atomics", HasAtomics)
      .Case("mutable-globals", HasMutableGlobals)
      .Case("multivalue", HasMultivalue)
      .Case("tail-call", HasTailCall)
      .Case("reference-types", HasReferenceTypes)
      .Default(false);
}

bool WebAssemblyTargetInfo::isValidCPUName(StringRef Name) const {
  return llvm::find(ValidCPUNames, Name) != std::end(ValidCPUNames);
}

void WebAssemblyTargetInfo::fillValidCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  Values.append(std::begin(ValidCPUNames), std::end(ValidCPUNames));
}

void WebAssemblyTargetInfo::setSIMDLevel(llvm::StringMap<bool> &Features,
                                         SIMDEnum Level, bool Enabled) {
  if (Enabled) {
    switch (Level) {
    case UnimplementedSIMD128:
      Features["unimplemented-simd128"] = true;
      LLVM_FALLTHROUGH;
    case SIMD128:
      Features["simd128"] = true;
      LLVM_FALLTHROUGH;
    case NoSIMD:
      break;
    }
    return;
  }

  switch (Level) {
  case NoSIMD:
  case SIMD128:
    Features["simd128"] = false;
    LLVM_FALLTHROUGH;
  case UnimplementedSIMD128:
    Features["unimplemented-simd128"] = false;
    break;
  }
}

void WebAssemblyTargetInfo::setFeatureEnabled(llvm::StringMap<bool> &Features,
                                              StringRef Name,
                                              bool Enabled) const {
  if (Name == "simd128")
    setSIMDLevel(Features, SIMD128, Enabled);
  else if (Name == "unimplemented-simd128")
    setSIMDLevel(Features, UnimplementedSIMD128, Enabled);
  else
    Features[Name] = Enabled;
}

bool WebAssemblyTargetInfo::initFeatureMap(
    llvm::StringMap<bool> &Features, DiagnosticsEngine &Diags, StringRef CPU,
    const std::vector<std::string> &FeaturesVec) const {
  if (CPU == "bleeding-edge") {
    Features["nontrapping-fptoint"] = true;
    Features["sign-ext"] = true;
    Features["bulk-memory"] = true;
    Features["atomics"] = true;
    Features["mutable-globals"] = true;
    Features["tail-call"] = true;
    setSIMDLevel(Features, SIMD128, true);
  }

  return TargetInfo::initFeatureMap(Features, Diags, CPU, FeaturesVec);
}

bool WebAssemblyTargetInfo::handleTargetFeatures(
    std::vector<std::string> &Features, DiagnosticsEngine &Diags) {
  for (const auto &Feature : Features) {
    if (Feature == "+simd128") {
      SIMDLevel = std::max(SIMDLevel, SIMD128);
      continue;
    }
    if (Feature == "-simd128") {
      SIMDLevel = std::min(SIMDLevel, SIMDEnum(SIMD128 - 1));
      continue;
    }
    if (Feature == "+unimplemented-simd128") {
      SIMDLevel = std::max(SIMDLevel, SIMDEnum(UnimplementedSIMD128));
      continue;
    }
    if (Feature == "-unimplemented-simd128") {
      SIMDLevel = std::min(SIMDLevel, SIMDEnum(UnimplementedSIMD128 - 1));
      continue;
    }
    if (Feature == "+nontrapping-fptoint") {
      HasNontrappingFPToInt = true;
      continue;
    }
    if (Feature == "-nontrapping-fptoint") {
      HasNontrappingFPToInt = false;
      continue;
    }
    if (Feature == "+sign-ext") {
      HasSignExt = true;
      continue;
    }
    if (Feature == "-sign-ext") {
      HasSignExt = false;
      continue;
    }
    if (Feature == "+exception-handling") {
      HasExceptionHandling = true;
      continue;
    }
    if (Feature == "-exception-handling") {
      HasExceptionHandling = false;
      continue;
    }
    if (Feature == "+bulk-memory") {
      HasBulkMemory = true;
      continue;
    }
    if (Feature == "-bulk-memory") {
      HasBulkMemory = false;
      continue;
    }
    if (Feature == "+atomics") {
      HasAtomics = true;
      continue;
    }
    if (Feature == "-atomics") {
      HasAtomics = false;
      continue;
    }
    if (Feature == "+mutable-globals") {
      HasMutableGlobals = true;
      continue;
    }
    if (Feature == "-mutable-globals") {
      HasMutableGlobals = false;
      continue;
    }
    if (Feature == "+multivalue") {
      HasMultivalue = true;
      continue;
    }
    if (Feature == "-multivalue") {
      HasMultivalue = false;
      continue;
    }
    if (Feature == "+tail-call") {
      HasTailCall = true;
      continue;
    }
    if (Feature == "-tail-call") {
      HasTailCall = false;
      continue;
    }
    if (Feature == "+reference-types") {
      HasReferenceTypes = true;
      continue;
    }
    if (Feature == "-reference-types") {
      HasReferenceTypes = false;
      continue;
    }

    Diags.Report(diag::err_opt_not_valid_with_opt)
        << Feature << "-target-feature";
    return false;
  }
  return true;
}

ArrayRef<Builtin::Info> WebAssemblyTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, fly::WebAssembly::LastTSBuiltin -
                                             Builtin::FirstTSBuiltin);
}
