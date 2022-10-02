//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGFunction.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"
#include "AST/ASTNameSpace.h"
#include "llvm/IR/DerivedTypes.h"

using namespace fly;

CodeGenClass::CodeGenClass(CodeGenModule *CGM, ASTClass *AST, bool isExternal) : CGM(CGM), AST(AST) {
    std::string Id = CodeGen::toIdentifier(AST->getName(), AST->getNameSpace()->getName());
    llvm::SmallVector<Type *, 4> StructTypes;
    for (auto &Field : AST->getVars()) {
        llvm::Type *FieldType = CGM->GenType(Field.second->getType());
        StructTypes.push_back(FieldType);
    }
    auto structType = llvm::StructType::create(CGM->LLVMCtx, StructTypes, Id);

}

//const llvm::StringRef CodeGenClass::getName() const {
//    return "";
//}
