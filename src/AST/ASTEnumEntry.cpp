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

using namespace fly;

ASTEnumEntry::ASTEnumEntry(const SourceLocation &Loc, ASTEnumType *Type, llvm::StringRef Name) :
        ASTVar(ASTVarKind::VAR_ENUM, Loc, Type, Name, nullptr) {

}

ASTEnum *ASTEnumEntry::getEnum() const {
    return Enum;
}

uint32_t ASTEnumEntry::getIndex() const {
    return Index;
}

CodeGenEnumEntry *ASTEnumEntry::getCodeGen() const {
    return CodeGen;
}

void ASTEnumEntry::setCodeGen(CodeGenEnumEntry *CGE) {
    this->CodeGen = CGE;
}

std::string ASTEnumEntry::print() const {
    return Enum->print() + "." + getName().data();
}

std::string ASTEnumEntry::str() const {
    return Logger("ASTEnumEntry").
            Super(ASTVar::str()).
            Attr("Index", (uint64_t) Index).
            Attr("Comment", Comment).
            End();
}
