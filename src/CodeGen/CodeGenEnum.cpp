//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClass.cpp - Code Generator Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenEnum.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenEnumEntry.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumEntry.h"

using namespace fly;

CodeGenEnum::CodeGenEnum(CodeGenModule *CGM, ASTEnum *Enum, bool isExternal) : CGM(CGM), AST(Enum), Type(CGM->getCodeGen()->Int32Ty) {

}

void CodeGenEnum::Generate() {
    for (auto &Entry : AST->getEntries()) {
        CodeGenEnumEntry *CGE = new CodeGenEnumEntry(CGM, Entry.getValue());
        Entry.getValue()->setCodeGen(CGE);
        Vars.insert(std::make_pair(Entry.getKey(), CGE));
    }
}

Type *CodeGenEnum::getType() const {
    return Type;
}
