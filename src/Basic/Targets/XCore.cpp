//===--- XCore.cpp - Implement XCore target feature support ---------------===//
//
// Part of the Fly Project, under the Apache License v2.0
//
//===----------------------------------------------------------------------===//
//
// This file implements XCore TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Basic/Targets/XCore.h"
#include "Basic/Builtins.h"

#include "Basic/TargetBuiltins.h"

using namespace fly;
using namespace fly::targets;

const Builtin::Info XCoreTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER)                                    \
  {#ID, TYPE, ATTRS, HEADER, ALL_LANGUAGES, nullptr},
#include "Basic/BuiltinsXCore.def"
};

ArrayRef<Builtin::Info> XCoreTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, fly::XCore::LastTSBuiltin -
                                         Builtin::FirstTSBuiltin);
}
