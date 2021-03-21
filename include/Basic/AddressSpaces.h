//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/AddressSpaces.h - Language-specific address spaces
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
/// \file
/// Provides definitions for the various language-specific address
/// spaces.
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_ADDRESSSPACES_H
#define LLVM_FLY_BASIC_ADDRESSSPACES_H

#include <cassert>

namespace fly {

/// Defines the address space values used by the address space qualifier
/// of QualType.
///
enum class LangAS : unsigned {
  // The default value 0 is the value used in QualType for the situation
  // where there is no address space qualifier.
  Default = 0,

  // OpenCL specific address spaces.
  // In OpenCL each l-value must have certain non-default address space, each
  // r-value must have no address space (i.e. the default address space). The
  // pointee of a pointer must have non-default address space.
  opencl_global,
  opencl_local,
  opencl_constant,
  opencl_private,
  opencl_generic,

  // CUDA specific address spaces.
  cuda_device,
  cuda_constant,
  cuda_shared,

  // Pointer size and extension address spaces.
  ptr32_sptr,
  ptr32_uptr,
  ptr64,

  // This denotes the count of language-specific address spaces and also
  // the offset added to the target-specific address spaces, which are usually
  // specified by address space attributes __attribute__(address_space(n))).
  FirstTargetAddressSpace
};

/// The type of a lookup table which maps from language-specific address spaces
/// to target-specific ones.
using LangASMap = unsigned[(unsigned)LangAS::FirstTargetAddressSpace];

/// \return whether \p AS is a target-specific address space rather than a
/// fly AST address space
inline bool isTargetAddressSpace(LangAS AS) {
  return (unsigned)AS >= (unsigned)LangAS::FirstTargetAddressSpace;
}

inline unsigned toTargetAddressSpace(LangAS AS) {
  assert(isTargetAddressSpace(AS));
  return (unsigned)AS - (unsigned)LangAS::FirstTargetAddressSpace;
}

inline LangAS getLangASFromTargetAS(unsigned TargetAS) {
  return static_cast<LangAS>((TargetAS) +
                             (unsigned)LangAS::FirstTargetAddressSpace);
}

inline bool isPtrSizeAddressSpace(LangAS AS) {
  return (AS == LangAS::ptr32_sptr || AS == LangAS::ptr32_uptr ||
          AS == LangAS::ptr64);
}

} // namespace fly

#endif // LLVM_FLY_BASIC_ADDRESSSPACES_H
