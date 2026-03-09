//===-------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTEnumEntry.cpp - AST Enum Entry implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTEnum.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTVisitor.h"
#include "Basic/Logger.h"

using namespace fly;

ASTEnumEntry::ASTEnumEntry(const SourceLocation &Loc, ASTEnum *Enum, llvm::StringRef Name) :
    ASTExpr(Loc, ASTExprKind::EXPR_VALUE), Enum(Enum), Name(Name) {
}

void ASTEnumEntry::accept(ASTVisitor &Visitor) {
    Visitor.visit(*this);
}

ASTEnum *ASTEnumEntry::getEnum() const {
    return Enum;
}

llvm::StringRef ASTEnumEntry::getName() const {
    return Name;
}

uint32_t ASTEnumEntry::getIndex() const {
    return Index;
}

void ASTEnumEntry::setIndex(uint32_t Idx) {
    Index = Idx;
}

SemaEnumEntry *ASTEnumEntry::getSema() const {
    return Sema;
}

void ASTEnumEntry::setSema(SemaEnumEntry *S) {
    this->Sema = S;
}

std::string ASTEnumEntry::str() const {
    return Logger("ASTEnumEntry").
           Attr("Enum", Enum->getName()).
           Attr("Name", Name).
           Attr("Index", (uint64_t) Index).
           End();
}
