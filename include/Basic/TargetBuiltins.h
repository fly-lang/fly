//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/TargetBuiltins.h - Target specific builtin IDs
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
///
/// \file
/// Enumerates target-specific builtins in their own namespaces within
/// namespace ::fly.
///
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_TARGETBUILTINS_H
#define LLVM_FLY_BASIC_TARGETBUILTINS_H

#include <stdint.h>
#include "Basic/Builtins.h"
#undef PPC

namespace fly {

  namespace NEON {
  enum {
    LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Basic/BuiltinsNEON.def"
    FirstTSBuiltin
  };
  }

  /// ARM builtins
  namespace ARM {
    enum {
      LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
      LastNEONBuiltin = NEON::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Basic/BuiltinsARM.def"
      LastTSBuiltin
    };
  }

  namespace SVE {
  enum {
    LastNEONBuiltin = NEON::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Basic/BuiltinsSVE.def"
    FirstTSBuiltin,
  };
  }

  /// AArch64 builtins
  namespace AArch64 {
  enum {
    LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
    LastNEONBuiltin = NEON::FirstTSBuiltin - 1,
    FirstSVEBuiltin = NEON::FirstTSBuiltin,
    LastSVEBuiltin = SVE::FirstTSBuiltin - 1,
  #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
  #include "Basic/BuiltinsAArch64.def"
    LastTSBuiltin
  };
  }

  /// BPF builtins
  namespace BPF {
  enum {
    LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
  #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
  #include "Basic/BuiltinsBPF.def"
    LastTSBuiltin
  };
  }

  /// PPC builtins
  namespace PPC {
    enum {
        LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Basic/BuiltinsPPC.def"
        LastTSBuiltin
    };
  }

  /// NVPTX builtins
  namespace NVPTX {
    enum {
        LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Basic/BuiltinsNVPTX.def"
        LastTSBuiltin
    };
  }

  /// AMDGPU builtins
  namespace AMDGPU {
  enum {
    LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
  #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
  #include "Basic/BuiltinsAMDGPU.def"
    LastTSBuiltin
  };
  }

  /// X86 builtins
  namespace X86 {
  enum {
    LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Basic/BuiltinsX86.def"
    FirstX86_64Builtin,
    LastX86CommonBuiltin = FirstX86_64Builtin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Basic/BuiltinsX86_64.def"
    LastTSBuiltin
  };
  }

  /// VE builtins
  namespace VE {
  enum { LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1, LastTSBuiltin };
  }

  /// Flags to identify the types for overloaded Neon builtins.
  ///
  /// These must be kept in sync with the flags in utils/TableGen/NeonEmitter.h.
  class NeonTypeFlags {
    enum {
      EltTypeMask = 0xf,
      UnsignedFlag = 0x10,
      QuadFlag = 0x20
    };
    uint32_t Flags;

  public:
    enum EltType {
      Int8,
      Int16,
      Int32,
      Int64,
      Poly8,
      Poly16,
      Poly64,
      Poly128,
      Float16,
      Float32,
      Float64,
      BFloat16
    };

    NeonTypeFlags(unsigned F) : Flags(F) {}
    NeonTypeFlags(EltType ET, bool IsUnsigned, bool IsQuad) : Flags(ET) {
      if (IsUnsigned)
        Flags |= UnsignedFlag;
      if (IsQuad)
        Flags |= QuadFlag;
    }

    EltType getEltType() const { return (EltType)(Flags & EltTypeMask); }
    bool isPoly() const {
      EltType ET = getEltType();
      return ET == Poly8 || ET == Poly16 || ET == Poly64;
    }
    bool isUnsigned() const { return (Flags & UnsignedFlag) != 0; }
    bool isQuad() const { return (Flags & QuadFlag) != 0; }
  };

//  /// Flags to identify the types for overloaded SVE builtins.
//  class SVETypeFlags {
//    uint64_t Flags;
//    unsigned EltTypeShift;
//    unsigned MemEltTypeShift;
//    unsigned MergeTypeShift;
//    unsigned SplatOperandMaskShift;
//
//  public:
//#define LLVM_GET_SVE_TYPEFLAGS
//#include "Basic/arm_sve_typeflags.inc"
//#undef LLVM_GET_SVE_TYPEFLAGS
//
//    enum EltType {
//#define LLVM_GET_SVE_ELTTYPES
//#include "Basic/arm_sve_typeflags.inc"
//#undef LLVM_GET_SVE_ELTTYPES
//    };
//
//    enum MemEltType {
//#define LLVM_GET_SVE_MEMELTTYPES
//#include "Basic/arm_sve_typeflags.inc"
//#undef LLVM_GET_SVE_MEMELTTYPES
//    };
//
//    enum MergeType {
//#define LLVM_GET_SVE_MERGETYPES
//#include "Basic/arm_sve_typeflags.inc"
//#undef LLVM_GET_SVE_MERGETYPES
//    };
//
//    enum ImmCheckType {
//#define LLVM_GET_SVE_IMMCHECKTYPES
//#include "Basic/arm_sve_typeflags.inc"
//#undef LLVM_GET_SVE_IMMCHECKTYPES
//    };
//
//    SVETypeFlags(uint64_t F) : Flags(F) {
//      EltTypeShift = llvm::countTrailingZeros(EltTypeMask);
//      MemEltTypeShift = llvm::countTrailingZeros(MemEltTypeMask);
//      MergeTypeShift = llvm::countTrailingZeros(MergeTypeMask);
//      SplatOperandMaskShift = llvm::countTrailingZeros(SplatOperandMask);
//    }
//
//    EltType getEltType() const {
//      return (EltType)((Flags & EltTypeMask) >> EltTypeShift);
//    }
//
//    MemEltType getMemEltType() const {
//      return (MemEltType)((Flags & MemEltTypeMask) >> MemEltTypeShift);
//    }
//
//    MergeType getMergeType() const {
//      return (MergeType)((Flags & MergeTypeMask) >> MergeTypeShift);
//    }
//
//    unsigned getSplatOperand() const {
//      return ((Flags & SplatOperandMask) >> SplatOperandMaskShift) - 1;
//    }
//
//    bool hasSplatOperand() const {
//      return Flags & SplatOperandMask;
//    }
//
//    bool isLoad() const { return Flags & IsLoad; }
//    bool isStore() const { return Flags & IsStore; }
//    bool isGatherLoad() const { return Flags & IsGatherLoad; }
//    bool isScatterStore() const { return Flags & IsScatterStore; }
//    bool isStructLoad() const { return Flags & IsStructLoad; }
//    bool isStructStore() const { return Flags & IsStructStore; }
//    bool isZExtReturn() const { return Flags & IsZExtReturn; }
//    bool isByteIndexed() const { return Flags & IsByteIndexed; }
//    bool isOverloadNone() const { return Flags & IsOverloadNone; }
//    bool isOverloadWhile() const { return Flags & IsOverloadWhile; }
//    bool isOverloadDefault() const { return !(Flags & OverloadKindMask); }
//    bool isOverloadWhileRW() const { return Flags & IsOverloadWhileRW; }
//    bool isOverloadCvt() const { return Flags & IsOverloadCvt; }
//    bool isPrefetch() const { return Flags & IsPrefetch; }
//    bool isReverseCompare() const { return Flags & ReverseCompare; }
//    bool isAppendSVALL() const { return Flags & IsAppendSVALL; }
//    bool isInsertOp1SVALL() const { return Flags & IsInsertOp1SVALL; }
//    bool isGatherPrefetch() const { return Flags & IsGatherPrefetch; }
//    bool isReverseUSDOT() const { return Flags & ReverseUSDOT; }
//    bool isUndef() const { return Flags & IsUndef; }
//    bool isTupleCreate() const { return Flags & IsTupleCreate; }
//    bool isTupleGet() const { return Flags & IsTupleGet; }
//    bool isTupleSet() const { return Flags & IsTupleSet; }
//
//    uint64_t getBits() const { return Flags; }
//    bool isFlagSet(uint64_t Flag) const { return Flags & Flag; }
//  };

  /// Hexagon builtins
  namespace Hexagon {
    enum {
        LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Basic/BuiltinsHexagon.def"
        LastTSBuiltin
    };
  }

  /// MIPS builtins
  namespace Mips {
    enum {
        LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Basic/BuiltinsMips.def"
        LastTSBuiltin
    };
  }

  /// XCore builtins
  namespace XCore {
    enum {
        LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Basic/BuiltinsXCore.def"
        LastTSBuiltin
    };
  }

  /// Le64 builtins
  namespace Le64 {
  enum {
    LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
  #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
  #include "Basic/BuiltinsLe64.def"
    LastTSBuiltin
  };
  }

  /// SystemZ builtins
  namespace SystemZ {
    enum {
        LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Basic/BuiltinsSystemZ.def"
        LastTSBuiltin
    };
  }

  /// WebAssembly builtins
  namespace WebAssembly {
    enum {
      LastTIBuiltin = fly::Builtin::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Basic/BuiltinsWebAssembly.def"
      LastTSBuiltin
    };
  }

  static constexpr uint64_t LargestBuiltinID = std::max<uint64_t>(
      {NEON::FirstTSBuiltin, ARM::LastTSBuiltin, SVE::FirstTSBuiltin,
       AArch64::LastTSBuiltin, BPF::LastTSBuiltin, PPC::LastTSBuiltin,
       NVPTX::LastTSBuiltin, AMDGPU::LastTSBuiltin, X86::LastTSBuiltin,
       Hexagon::LastTSBuiltin, Mips::LastTSBuiltin, XCore::LastTSBuiltin,
       Le64::LastTSBuiltin, SystemZ::LastTSBuiltin,
       WebAssembly::LastTSBuiltin});

} // end namespace fly.

#endif
