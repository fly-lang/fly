//===--- CodeGenOptions.cpp -----------------------------------------------===//
//
// Part of the Fly Project, under the Apache License v2.0
//
//===----------------------------------------------------------------------===//

#include "Basic/CodeGenOptions.h"
#include <string.h>

namespace fly {

CodeGenOptions::CodeGenOptions() {
#define CODEGENOPT(Name, Bits, Default) Name = Default;
#define ENUM_CODEGENOPT(Name, Type, Bits, Default) set##Name(Default);
#include "Basic/CodeGenOptions.def"

  RelocationModel = llvm::Reloc::PIC_;
  memcpy(CoverageVersion, "402*", 4);
}

bool CodeGenOptions::isNoBuiltinFunc(const char *Name) const {
  StringRef FuncName(Name);
  for (unsigned i = 0, e = NoBuiltinFuncs.size(); i != e; ++i)
    if (FuncName.equals(NoBuiltinFuncs[i]))
      return true;
  return false;
}

}  // end namespace clang
