//===--- AArch64.cpp - Implement AArch64 target feature support -----------===//
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
//
// This file implements AArch64 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Basic/Targets/AArch64.h"
#include "Basic/TargetBuiltins.h"
#include "Basic/TargetInfo.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/AArch64TargetParser.h"

using namespace fly;
using namespace fly::targets;

const Builtin::Info AArch64TargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
   {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#include "Basic/BuiltinsNEON.def"

#define BUILTIN(ID, TYPE, ATTRS)                                               \
   {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#define LANGBUILTIN(ID, TYPE, ATTRS, LANG)                                     \
  {#ID, TYPE, ATTRS, nullptr, LANG, nullptr},
#define TARGET_HEADER_BUILTIN(ID, TYPE, ATTRS, HEADER, LANGS, FEATURE)         \
  {#ID, TYPE, ATTRS, HEADER, LANGS, FEATURE},
#include "Basic/BuiltinsAArch64.def"
};

AArch64TargetInfo::AArch64TargetInfo(const llvm::Triple &Triple,
                                     const TargetOptions &Opts)
    : TargetInfo(Triple), ABI("aapcs") {
  if (getTriple().isOSOpenBSD()) {
    Int64Type = SignedLongLong;
    IntMaxType = SignedLongLong;
  } else {
    if (!getTriple().isOSDarwin() && !getTriple().isOSNetBSD())
      WCharType = UnsignedInt;

    Int64Type = SignedLong;
    IntMaxType = SignedLong;
  }

  // All AArch64 implementations support ARMv8 FP, which makes half a legal type.
  HasLegalHalfType = true;
  HasFloat16 = true;

  if (Triple.isArch64Bit())
    LongWidth = LongAlign = PointerWidth = PointerAlign = 64;
  else
    LongWidth = LongAlign = PointerWidth = PointerAlign = 32;

  MaxVectorAlign = 128;
  MaxAtomicInlineWidth = 128;
  MaxAtomicPromoteWidth = 128;

  LongDoubleWidth = LongDoubleAlign = SuitableAlign = 128;
  LongDoubleFormat = &llvm::APFloat::IEEEquad();

  // Make __builtin_ms_va_list available.
  HasBuiltinMSVaList = true;

  // Make the SVE types available.  Note that this deliberately doesn't
  // depend on SveMode, since in principle it should be possible to turn
  // SVE on and off within a translation unit.  It should also be possible
  // to compile the global declaration:
  //
  // __SVInt8_t *ptr;
  //
  // even without SVE.
  HasAArch64SVETypes = true;

  // {} in inline assembly are neon specifiers, not assembly variant
  // specifiers.
  NoAsmVariants = true;

  // AAPCS gives rules for bitfields. 7.1.7 says: "The container type
  // contributes to the alignment of the containing aggregate in the same way
  // a plain (non bit-field) member of that type would, without exception for
  // zero-sized or anonymous bit-fields."
  assert(UseBitFieldTypeAlignment && "bitfields affect type alignment");
  UseZeroLengthBitfieldAlignment = true;

  // AArch64 targets default to using the ARM C++ ABI.
  TheCXXABI.set(TargetCXXABI::GenericAArch64);

  if (Triple.getOS() == llvm::Triple::Linux)
    this->MCountName = "\01_mcount";
  else if (Triple.getOS() == llvm::Triple::UnknownOS)
    this->MCountName =
        Opts.EABIVersion == llvm::EABI::GNU ? "\01_mcount" : "mcount";
}

StringRef AArch64TargetInfo::getABI() const { return ABI; }

bool AArch64TargetInfo::setABI(const std::string &Name) {
  if (Name != "aapcs" && Name != "darwinpcs")
    return false;

  ABI = Name;
  return true;
}

bool AArch64TargetInfo::validateBranchProtection(StringRef Spec,
                                                 BranchProtectionInfo &BPI,
                                                 StringRef &Err) const {
  llvm::AArch64::ParsedBranchProtection PBP;
  if (!llvm::AArch64::parseBranchProtection(Spec, PBP, Err))
    return false;

  BPI.SignReturnAddr =
      llvm::StringSwitch<CodeGenOptions::SignReturnAddressScope>(PBP.Scope)
          .Case("non-leaf", CodeGenOptions::SignReturnAddressScope::NonLeaf)
          .Case("all", CodeGenOptions::SignReturnAddressScope::All)
          .Default(CodeGenOptions::SignReturnAddressScope::None);

  if (PBP.Key == "a_key")
    BPI.SignKey = CodeGenOptions::SignReturnAddressKeyValue::AKey;
  else
    BPI.SignKey = CodeGenOptions::SignReturnAddressKeyValue::BKey;

  BPI.BranchTargetEnforcement = PBP.BranchTargetEnforcement;
  return true;
}

bool AArch64TargetInfo::isValidCPUName(StringRef Name) const {
  return Name == "generic" ||
         llvm::AArch64::parseCPUArch(Name) != llvm::AArch64::ArchKind::INVALID;
}

bool AArch64TargetInfo::setCPU(const std::string &Name) {
  return isValidCPUName(Name);
}

void AArch64TargetInfo::fillValidCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  llvm::AArch64::fillValidCPUArchList(Values);
}

ArrayRef<Builtin::Info> AArch64TargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, fly::AArch64::LastTSBuiltin -
                                         Builtin::FirstTSBuiltin);
}

bool AArch64TargetInfo::hasFeature(StringRef Feature) const {
  return Feature == "aarch64" || Feature == "arm64" || Feature == "arm" ||
         (Feature == "neon" && (FPU & NeonMode)) ||
         (Feature == "sve" && (FPU & SveMode));
}

bool AArch64TargetInfo::handleTargetFeatures(std::vector<std::string> &Features,
                                             DiagnosticsEngine &Diags) {
  FPU = FPUMode;
  HasCRC = false;
  HasCrypto = false;
  HasUnaligned = true;
  HasFullFP16 = false;
  HasDotProd = false;
  HasFP16FML = false;
  HasMTE = false;
  HasTME = false;
  ArchKind = llvm::AArch64::ArchKind::ARMV8A;

  for (const auto &Feature : Features) {
    if (Feature == "+neon")
      FPU |= NeonMode;
    if (Feature == "+sve")
      FPU |= SveMode;
    if (Feature == "+crc")
      HasCRC = true;
    if (Feature == "+crypto")
      HasCrypto = true;
    if (Feature == "+strict-align")
      HasUnaligned = false;
    if (Feature == "+v8.1a")
      ArchKind = llvm::AArch64::ArchKind::ARMV8_1A;
    if (Feature == "+v8.2a")
      ArchKind = llvm::AArch64::ArchKind::ARMV8_2A;
    if (Feature == "+v8.3a")
      ArchKind = llvm::AArch64::ArchKind::ARMV8_3A;
    if (Feature == "+v8.4a")
      ArchKind = llvm::AArch64::ArchKind::ARMV8_4A;
    if (Feature == "+v8.5a")
      ArchKind = llvm::AArch64::ArchKind::ARMV8_5A;
    if (Feature == "+fullfp16")
      HasFullFP16 = true;
    if (Feature == "+dotprod")
      HasDotProd = true;
    if (Feature == "+fp16fml")
      HasFP16FML = true;
    if (Feature == "+mte")
      HasMTE = true;
    if (Feature == "+tme")
      HasTME = true;
  }

  setDataLayout();

  return true;
}

TargetInfo::CallingConvCheckResult
AArch64TargetInfo::checkCallingConvention(CallingConv CC) const {
  switch (CC) {
  case CC_C:
  case CC_Swift:
  case CC_PreserveMost:
  case CC_PreserveAll:
  case CC_OpenCLKernel:
  case CC_AArch64VectorCall:
  case CC_Win64:
    return CCCR_OK;
  default:
    return CCCR_Warning;
  }
}

bool AArch64TargetInfo::isCLZForZeroUndef() const { return false; }

TargetInfo::BuiltinVaListKind AArch64TargetInfo::getBuiltinVaListKind() const {
  return TargetInfo::AArch64ABIBuiltinVaList;
}

const char *const AArch64TargetInfo::GCCRegNames[] = {
    // 32-bit Integer registers
    "w0", "w1", "w2", "w3", "w4", "w5", "w6", "w7", "w8", "w9", "w10", "w11",
    "w12", "w13", "w14", "w15", "w16", "w17", "w18", "w19", "w20", "w21", "w22",
    "w23", "w24", "w25", "w26", "w27", "w28", "w29", "w30", "wsp",

    // 64-bit Integer registers
    "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11",
    "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19", "x20", "x21", "x22",
    "x23", "x24", "x25", "x26", "x27", "x28", "fp", "lr", "sp",

    // 32-bit floating point regsisters
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",
    "s23", "s24", "s25", "s26", "s27", "s28", "s29", "s30", "s31",

    // 64-bit floating point regsisters
    "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "d10", "d11",
    "d12", "d13", "d14", "d15", "d16", "d17", "d18", "d19", "d20", "d21", "d22",
    "d23", "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31",

    // Neon vector registers
    "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11",
    "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22",
    "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",

    // SVE vector registers
    "z0",  "z1",  "z2",  "z3",  "z4",  "z5",  "z6",  "z7",  "z8",  "z9",  "z10",
    "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21",
    "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31",

    // SVE predicate registers
    "p0",  "p1",  "p2",  "p3",  "p4",  "p5",  "p6",  "p7",  "p8",  "p9",  "p10",
    "p11", "p12", "p13", "p14", "p15"
};

ArrayRef<const char *> AArch64TargetInfo::getGCCRegNames() const {
  return llvm::makeArrayRef(GCCRegNames);
}

const TargetInfo::GCCRegAlias AArch64TargetInfo::GCCRegAliases[] = {
    {{"w31"}, "wsp"},
    {{"x31"}, "sp"},
    // GCC rN registers are aliases of xN registers.
    {{"r0"}, "x0"},
    {{"r1"}, "x1"},
    {{"r2"}, "x2"},
    {{"r3"}, "x3"},
    {{"r4"}, "x4"},
    {{"r5"}, "x5"},
    {{"r6"}, "x6"},
    {{"r7"}, "x7"},
    {{"r8"}, "x8"},
    {{"r9"}, "x9"},
    {{"r10"}, "x10"},
    {{"r11"}, "x11"},
    {{"r12"}, "x12"},
    {{"r13"}, "x13"},
    {{"r14"}, "x14"},
    {{"r15"}, "x15"},
    {{"r16"}, "x16"},
    {{"r17"}, "x17"},
    {{"r18"}, "x18"},
    {{"r19"}, "x19"},
    {{"r20"}, "x20"},
    {{"r21"}, "x21"},
    {{"r22"}, "x22"},
    {{"r23"}, "x23"},
    {{"r24"}, "x24"},
    {{"r25"}, "x25"},
    {{"r26"}, "x26"},
    {{"r27"}, "x27"},
    {{"r28"}, "x28"},
    {{"r29", "x29"}, "fp"},
    {{"r30", "x30"}, "lr"},
    // The S/D/Q and W/X registers overlap, but aren't really aliases; we
    // don't want to substitute one of these for a different-sized one.
};

ArrayRef<TargetInfo::GCCRegAlias> AArch64TargetInfo::getGCCRegAliases() const {
  return llvm::makeArrayRef(GCCRegAliases);
}

bool AArch64TargetInfo::validateAsmConstraint(
    const char *&Name, TargetInfo::ConstraintInfo &Info) const {
  switch (*Name) {
  default:
    return false;
  case 'w': // Floating point and SIMD registers (V0-V31)
    Info.setAllowsRegister();
    return true;
  case 'I': // Constant that can be used with an ADD instruction
  case 'J': // Constant that can be used with a SUB instruction
  case 'K': // Constant that can be used with a 32-bit logical instruction
  case 'L': // Constant that can be used with a 64-bit logical instruction
  case 'M': // Constant that can be used as a 32-bit MOV immediate
  case 'N': // Constant that can be used as a 64-bit MOV immediate
  case 'Y': // Floating point constant zero
  case 'Z': // Integer constant zero
    return true;
  case 'Q': // A memory reference with base register and no offset
    Info.setAllowsMemory();
    return true;
  case 'S': // A symbolic address
    Info.setAllowsRegister();
    return true;
  case 'U':
    // Ump: A memory address suitable for ldp/stp in SI, DI, SF and DF modes.
    // Utf: A memory address suitable for ldp/stp in TF mode.
    // Usa: An absolute symbolic address.
    // Ush: The high part (bits 32:12) of a pc-relative symbolic address.
    llvm_unreachable("FIXME: Unimplemented support for U* constraints.");
  case 'z': // Zero register, wzr or xzr
    Info.setAllowsRegister();
    return true;
  case 'x': // Floating point and SIMD registers (V0-V15)
    Info.setAllowsRegister();
    return true;
  }
  return false;
}

bool AArch64TargetInfo::validateConstraintModifier(
    StringRef Constraint, char Modifier, unsigned Size,
    std::string &SuggestedModifier) const {
  // Strip off constraint modifiers.
  while (Constraint[0] == '=' || Constraint[0] == '+' || Constraint[0] == '&')
    Constraint = Constraint.substr(1);

  switch (Constraint[0]) {
  default:
    return true;
  case 'z':
  case 'r': {
    switch (Modifier) {
    case 'x':
    case 'w':
      // For now assume that the person knows what they're
      // doing with the modifier.
      return true;
    default:
      // By default an 'r' constraint will be in the 'x'
      // registers.
      if (Size == 64)
        return true;

      SuggestedModifier = "w";
      return false;
    }
  }
  }
}

const char *AArch64TargetInfo::getClobbers() const { return ""; }

int AArch64TargetInfo::getEHDataRegisterNumber(unsigned RegNo) const {
  if (RegNo == 0)
    return 0;
  if (RegNo == 1)
    return 1;
  return -1;
}

bool AArch64TargetInfo::hasInt128Type() const { return true; }

AArch64leTargetInfo::AArch64leTargetInfo(const llvm::Triple &Triple,
                                         const TargetOptions &Opts)
    : AArch64TargetInfo(Triple, Opts) {}

void AArch64leTargetInfo::setDataLayout() {
  if (getTriple().isOSBinFormatMachO()) {
    if(getTriple().isArch32Bit())
      resetDataLayout("e-m:o-p:32:32-i64:64-i128:128-n32:64-S128");
    else
      resetDataLayout("e-m:o-i64:64-i128:128-n32:64-S128");
  } else
    resetDataLayout("e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128");
}

AArch64beTargetInfo::AArch64beTargetInfo(const llvm::Triple &Triple,
                                         const TargetOptions &Opts)
    : AArch64TargetInfo(Triple, Opts) {}

void AArch64beTargetInfo::setDataLayout() {
  assert(!getTriple().isOSBinFormatMachO());
  resetDataLayout("E-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128");
}

WindowsARM64TargetInfo::WindowsARM64TargetInfo(const llvm::Triple &Triple,
                                               const TargetOptions &Opts)
    : WindowsTargetInfo<AArch64leTargetInfo>(Triple, Opts), Triple(Triple) {

  // This is an LLP64 platform.
  // int:4, long:4, long long:8, long double:8.
  IntWidth = IntAlign = 32;
  LongWidth = LongAlign = 32;
  DoubleAlign = LongLongAlign = 64;
  LongDoubleWidth = LongDoubleAlign = 64;
  LongDoubleFormat = &llvm::APFloat::IEEEdouble();
  IntMaxType = SignedLongLong;
  Int64Type = SignedLongLong;
  SizeType = UnsignedLongLong;
  PtrDiffType = SignedLongLong;
  IntPtrType = SignedLongLong;
}

void WindowsARM64TargetInfo::setDataLayout() {
  resetDataLayout("e-m:w-p:64:64-i32:32-i64:64-i128:128-n32:64-S128");
}

TargetInfo::BuiltinVaListKind
WindowsARM64TargetInfo::getBuiltinVaListKind() const {
  return TargetInfo::CharPtrBuiltinVaList;
}

TargetInfo::CallingConvCheckResult
WindowsARM64TargetInfo::checkCallingConvention(CallingConv CC) const {
  switch (CC) {
  case CC_X86StdCall:
  case CC_X86ThisCall:
  case CC_X86FastCall:
  case CC_X86VectorCall:
    return CCCR_Ignore;
  case CC_C:
  case CC_OpenCLKernel:
  case CC_PreserveMost:
  case CC_PreserveAll:
  case CC_Swift:
  case CC_Win64:
    return CCCR_OK;
  default:
    return CCCR_Warning;
  }
}

MicrosoftARM64TargetInfo::MicrosoftARM64TargetInfo(const llvm::Triple &Triple,
                                                   const TargetOptions &Opts)
    : WindowsARM64TargetInfo(Triple, Opts) {
  TheCXXABI.set(TargetCXXABI::Microsoft);
}

TargetInfo::CallingConvKind
MicrosoftARM64TargetInfo::getCallingConvKind(bool ClangABICompat4) const {
  return CCK_MicrosoftWin64;
}

unsigned MicrosoftARM64TargetInfo::getMinGlobalAlign(uint64_t TypeSize) const {
  unsigned Align = WindowsARM64TargetInfo::getMinGlobalAlign(TypeSize);

  // MSVC does size based alignment for arm64 based on alignment section in
  // below document, replicate that to keep alignment consistent with object
  // files compiled by MSVC.
  // https://docs.microsoft.com/en-us/cpp/build/arm64-windows-abi-conventions
  if (TypeSize >= 512) {              // TypeSize >= 64 bytes
    Align = std::max(Align, 128u);    // align type at least 16 bytes
  } else if (TypeSize >= 64) {        // TypeSize >= 8 bytes
    Align = std::max(Align, 64u);     // align type at least 8 butes
  } else if (TypeSize >= 16) {        // TypeSize >= 2 bytes
    Align = std::max(Align, 32u);     // align type at least 4 bytes
  }
  return Align;
}

MinGWARM64TargetInfo::MinGWARM64TargetInfo(const llvm::Triple &Triple,
                                           const TargetOptions &Opts)
    : WindowsARM64TargetInfo(Triple, Opts) {
  TheCXXABI.set(TargetCXXABI::GenericAArch64);
}

DarwinAArch64TargetInfo::DarwinAArch64TargetInfo(const llvm::Triple &Triple,
                                                 const TargetOptions &Opts)
    : DarwinTargetInfo<AArch64leTargetInfo>(Triple, Opts) {
  Int64Type = SignedLongLong;
  if (getTriple().isArch32Bit())
    IntMaxType = SignedLongLong;

  WCharType = SignedInt;
  UseSignedCharForObjCBool = false;

  LongDoubleWidth = LongDoubleAlign = SuitableAlign = 64;
  LongDoubleFormat = &llvm::APFloat::IEEEdouble();

  UseZeroLengthBitfieldAlignment = false;

  if (getTriple().isArch32Bit()) {
    UseBitFieldTypeAlignment = false;
    ZeroLengthBitfieldBoundary = 32;
    UseZeroLengthBitfieldAlignment = true;
    TheCXXABI.set(TargetCXXABI::WatchOS);
  } else
    TheCXXABI.set(TargetCXXABI::iOS64);
}

TargetInfo::BuiltinVaListKind
DarwinAArch64TargetInfo::getBuiltinVaListKind() const {
  return TargetInfo::CharPtrBuiltinVaList;
}

// 64-bit RenderScript is aarch64
RenderScript64TargetInfo::RenderScript64TargetInfo(const llvm::Triple &Triple,
                                                   const TargetOptions &Opts)
    : AArch64leTargetInfo(llvm::Triple("aarch64", Triple.getVendorName(),
                                       Triple.getOSName(),
                                       Triple.getEnvironmentName()),
                          Opts) {
  IsRenderScriptTarget = true;
}
