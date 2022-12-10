//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClassFunction.cpp - Code Generator Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClassFunction.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTClassFunction.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTClass.h"

using namespace fly;

CodeGenClassFunction::CodeGenClassFunction(CodeGenModule *CGM, ASTClassFunction *AST) : CodeGenFunctionBase(CGM, AST) {
    std::string Id = CodeGen::toIdentifier(AST->getName(), AST->getClass()->getNameSpace()->getName(), AST->getClass()->getName());
    Fn->setName(Id);
}