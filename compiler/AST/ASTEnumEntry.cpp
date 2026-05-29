//===-------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTEnumEntry.cpp - AST enum entry implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTEnum.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTVisitor.h"
#include "Sema/Symbol.h"
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

Symbol *ASTEnumEntry::getSymbol() const {
    return Sym;
}

void ASTEnumEntry::setSymbol(Symbol *S) {
    Sym = S;
}

std::string ASTEnumEntry::str() const {
    return Logger("ASTEnumEntry")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("Enum", Enum->getName())
        .Attr("Name", Name)
        .Attr("Index", (uint64_t) Index)
        .Attr("Symbol", Sym ? Sym->getName() : std::string("null"))
        .End();
}
