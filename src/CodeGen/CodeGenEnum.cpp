//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClass.cpp - Code Generator Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenEnum.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"

using namespace fly;

CodeGenEnum::CodeGenEnum(CodeGenModule *CGM, ASTClass *Class, bool isExternal) : CGM(CGM), AST(Class) {
    uint64_t Index = 0;
    for (auto &Entry : Class->getVars()) {
        ASTClassVar *Var = Entry.getValue();
        Vars.insert(std::make_pair(Var->getName(), llvm::ConstantInt::get(CGM->Int32Ty, Index)));
    }
}
