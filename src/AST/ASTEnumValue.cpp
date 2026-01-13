//===-------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTEnumValue.cpp - AST Enum Value implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTEnum.h"
#include "AST/ASTEnumValue.h"
#include "AST/ASTVisitor.h"
#include "Basic/Logger.h"

using namespace fly;

ASTEnumValue::ASTEnumValue(const SourceLocation &Loc, ASTEnum *Enum, llvm::StringRef Name) :
    ASTValue(ASTValueKind::VAL_ENUM, Loc), Enum(Enum), Name(Name) {
}

void ASTEnumValue::accept(ASTVisitor &Visitor) {
    Visitor.visit(*this);
}

ASTEnum *ASTEnumValue::getEnum() const {
    return Enum;
}

llvm::StringRef ASTEnumValue::getName() const {
    return Name;
}

uint32_t ASTEnumValue::getIndex() const {
    return Index;
}

void ASTEnumValue::setIndex(uint32_t Idx) {
    Index = Idx;
}

SemaEnumValue *ASTEnumValue::getSema() const {
    return Sema;
}

void ASTEnumValue::setSema(SemaEnumValue *S) {
    this->Sema = S;
}

std::string ASTEnumValue::str() const {
    return Logger("ASTEnumValue").
           Attr("Enum", Enum->getName()).
           Attr("Name", Name).
           Attr("Index", (uint64_t) Index).
           End();
}
