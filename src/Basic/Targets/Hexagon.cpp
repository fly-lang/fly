//===--- Hexagon.cpp - Implement Hexagon target feature support -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements Hexagon TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Hexagon.h"
#include "Basic/Targets.h"
#include "Basic/TargetBuiltins.h"
#include "llvm/ADT/StringSwitch.h"

using namespace fly;
using namespace fly::targets;

bool HexagonTargetInfo::initFeatureMap(
    llvm::StringMap<bool> &Features, DiagnosticsEngine &Diags, StringRef CPU,
    const std::vector<std::string> &FeaturesVec) const {
  if (isTinyCore())
    Features["audio"] = true;

  StringRef CPUFeature = CPU;
  CPUFeature.consume_front("hexagon");
  CPUFeature.consume_back("t");
  Features[CPUFeature] = true;

  Features["long-calls"] = false;

  return TargetInfo::initFeatureMap(Features, Diags, CPU, FeaturesVec);
}

bool HexagonTargetInfo::handleTargetFeatures(std::vector<std::string> &Features,
                                             DiagnosticsEngine &Diags) {
  for (auto &F : Features) {
    if (F == "+hvx-length64b")
      HasHVX = HasHVX64B = true;
    else if (F == "+hvx-length128b")
      HasHVX = HasHVX128B = true;
    else if (F.find("+hvxv") != std::string::npos) {
      HasHVX = true;
      HVXVersion = F.substr(std::string("+hvxv").length());
    } else if (F == "-hvx")
      HasHVX = HasHVX64B = HasHVX128B = false;
    else if (F == "+long-calls")
      UseLongCalls = true;
    else if (F == "-long-calls")
      UseLongCalls = false;
    else if (F == "+audio")
      HasAudio = true;
  }
  return true;
}

const char *const HexagonTargetInfo::GCCRegNames[] = {
    "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",  "r8",
    "r9",  "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17",
    "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26",
    "r27", "r28", "r29", "r30", "r31", "p0",  "p1",  "p2",  "p3",
    "sa0", "lc0", "sa1", "lc1", "m0",  "m1",  "usr", "ugp",
    "r1:0", "r3:2", "r5:4", "r7:6", "r9:8", "r11:10", "r13:12", "r15:14",
    "r17:16", "r19:18", "r21:20", "r23:22", "r25:24", "r27:26", "r29:28",
    "r31:30"
};

ArrayRef<const char *> HexagonTargetInfo::getGCCRegNames() const {
  return llvm::makeArrayRef(GCCRegNames);
}

const TargetInfo::GCCRegAlias HexagonTargetInfo::GCCRegAliases[] = {
    {{"sp"}, "r29"},
    {{"fp"}, "r30"},
    {{"lr"}, "r31"},
};

ArrayRef<TargetInfo::GCCRegAlias> HexagonTargetInfo::getGCCRegAliases() const {
  return llvm::makeArrayRef(GCCRegAliases);
}

const Builtin::Info HexagonTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER)                                    \
  {#ID, TYPE, ATTRS, HEADER, ALL_LANGUAGES, nullptr},
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE)                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, FEATURE},
#include "Basic/BuiltinsHexagon.def"
};

bool HexagonTargetInfo::hasFeature(StringRef Feature) const {
  std::string VS = "hvxv" + HVXVersion;
  if (Feature == VS)
    return true;

  return llvm::StringSwitch<bool>(Feature)
      .Case("hexagon", true)
      .Case("hvx", HasHVX)
      .Case("hvx-length64b", HasHVX64B)
      .Case("hvx-length128b", HasHVX128B)
      .Case("long-calls", UseLongCalls)
      .Case("audio", HasAudio)
      .Default(false);
}

struct CPUSuffix {
  llvm::StringLiteral Name;
  llvm::StringLiteral Suffix;
};

static constexpr CPUSuffix Suffixes[] = {
    {{"hexagonv5"},  {"5"}},  {{"hexagonv55"},  {"55"}},
    {{"hexagonv60"}, {"60"}}, {{"hexagonv62"},  {"62"}},
    {{"hexagonv65"}, {"65"}}, {{"hexagonv66"},  {"66"}},
    {{"hexagonv67"}, {"67"}}, {{"hexagonv67t"}, {"67t"}},
};

const char *HexagonTargetInfo::getHexagonCPUSuffix(StringRef Name) {
  const CPUSuffix *Item = llvm::find_if(
      Suffixes, [Name](const CPUSuffix &S) { return S.Name == Name; });
  if (Item == std::end(Suffixes))
    return nullptr;
  return Item->Suffix.data();
}

void HexagonTargetInfo::fillValidCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  for (const CPUSuffix &Suffix : Suffixes)
    Values.push_back(Suffix.Name);
}

ArrayRef<Builtin::Info> HexagonTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, fly::Hexagon::LastTSBuiltin -
                                             Builtin::FirstTSBuiltin);
}
