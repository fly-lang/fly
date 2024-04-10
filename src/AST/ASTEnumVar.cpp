//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTEnumVar.cpp - Enum Field implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTEnumVar.h"
#include "AST/ASTEnum.h"

using namespace fly;

ASTEnumVar::ASTEnumVar(ASTEnum *Enum, const SourceLocation &Loc, llvm::StringRef Name) :
        VarKind(ASTVarKind::VAR_ENUM), Name(Name), Loc(Loc), Enum(Enum) {

}

const SourceLocation &ASTEnumVar::getLocation() const {
    return Loc;
}

llvm::StringRef ASTEnumVar::getName() const {
    return Name;
}

ASTVarKind ASTEnumVar::getVarKind() {
    return VarKind;
}

ASTEnum *ASTEnumVar::getEnum() const {
    return Enum;
}

uint32_t ASTEnumVar::getIndex() const {
    return Index;
}

ASTType *ASTEnumVar::getType() const {
    return (ASTType *) Enum->getType();
}

llvm::StringRef ASTEnumVar::getComment() const {
    return Comment;
}

CodeGenEnumEntry *ASTEnumVar::getCodeGen() const {
    return CodeGen;
}

void ASTEnumVar::setCodeGen(CodeGenEnumEntry *CGE) {
    this->CodeGen = CGE;
}

std::string ASTEnumVar::print() const {
    return Enum->print() + "." + Name.data();
}

std::string ASTEnumVar::str() const {
    return Logger("ASTEnumVar").
            Attr("Name", Name).
            Attr("Index", (uint64_t) Index).
            Attr("VarKind", (uint64_t) VarKind).
            Attr("Comment", Comment).
            End();
}
