//===----------------------------------------------------------------------===//
// CodeGen/CodeGen.cpp - Code Generator implementation
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#include "CodeGen/CodeGen.h"

using namespace fly;

CodeGen::CodeGen(ASTContext &Context) : Context(Context) {

}

bool CodeGen::execute() const {
    return false;
}
