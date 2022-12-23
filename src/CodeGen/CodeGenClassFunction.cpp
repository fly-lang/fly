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

}

Function *CodeGenClassFunction::Create() {
    Fn = CodeGenFunctionBase::Create();

    ASTClass *Class = ((ASTClassFunction *) getAST())->getClass();
    std::string Id = CodeGen::toIdentifier(getAST()->getName(), Class->getNameSpace()->getName(), Class->getName());
    Fn->setName(Id);

    GenBody();

    return Fn;
}
