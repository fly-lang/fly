//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTEnumEntry.cpp - AST Enum Entry implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTEnumEntry.h"
#include "AST/ASTEnum.h"
#include "CodeGen/CodeGenEnumEntry.h"

using namespace fly;

static SmallVector<ASTScope *, 8> CreateScopes() {

}

ASTEnumEntry::ASTEnumEntry(const SourceLocation &Loc, ASTEnumType *Type, llvm::StringRef Name,
                           SmallVector<ASTScope *, 8> &Scopes) :
        ASTVar(ASTVarKind::VAR_ENUM, Loc, Type, Name, Scopes) {
}

ASTEnum *ASTEnumEntry::getEnum() const {
    return Enum;
}

uint32_t ASTEnumEntry::getIndex() const {
    return Index;
}

CodeGenVarBase *ASTEnumEntry::getCodeGen() const {
    return CodeGen;
}

void ASTEnumEntry::setCodeGen(CodeGenEnumEntry *CG) {
    CodeGen = CG;
}

std::string ASTEnumEntry::str() const {
    return Logger("ASTEnumEntry").
            Super(ASTVar::str()).
            Attr("Index", (uint64_t) Index).
            Attr("Comment", Comment).
            End();
}
