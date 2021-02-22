//===--- XRayInstr.cpp ------------------------------------------*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
//
// This is part of XRay, a function call instrumentation system.
//
//===----------------------------------------------------------------------===//

#include "Basic/XRayInstr.h"
#include "llvm/ADT/StringSwitch.h"

namespace fly {

XRayInstrMask parseXRayInstrValue(StringRef Value) {
  XRayInstrMask ParsedKind = llvm::StringSwitch<XRayInstrMask>(Value)
                                 .Case("all", XRayInstrKind::All)
                                 .Case("custom", XRayInstrKind::Custom)
                                 .Case("function", XRayInstrKind::Function)
                                 .Case("typed", XRayInstrKind::Typed)
                                 .Case("none", XRayInstrKind::None)
                                 .Default(XRayInstrKind::None);
  return ParsedKind;
}

} // namespace clang
