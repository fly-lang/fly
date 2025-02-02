//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentifier.cpp - AST Identifier implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTIdentifier.h"

using namespace fly;


ASTIdentifier::ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTRefKind Kind) :
        ASTBase(Loc, ASTKind::AST_IDENTIFIER), Name(Name), RefKind(Kind), Resolved(false) {
    FullName = Name.data();
}


ASTIdentifier::~ASTIdentifier() {
    delete Parent;
}

llvm::StringRef ASTIdentifier::getName() const {
    return Name;
}

std::string ASTIdentifier::getFullName() const {
    return FullName;
}

bool ASTIdentifier::isResolved() const {
    return Resolved;
}

bool ASTIdentifier::isType() const {
    return RefKind == ASTRefKind::REF_TYPE;
}

bool ASTIdentifier::isCall() const {
    return RefKind == ASTRefKind::REF_CALL;
}

bool ASTIdentifier::isVarRef() const {
    return RefKind == ASTRefKind::REF_VAR;
}

ASTRefKind ASTIdentifier::getRefKind() const {
    return RefKind;
}

void ASTIdentifier::AddChild(ASTIdentifier *Identifier) {
    Child = Identifier;
    Child->Parent = this;
    Child->FullName = FullName.append(".").append(Child->Name.data());
}

ASTIdentifier *ASTIdentifier::getParent() const {
    return Parent;
}

ASTIdentifier *ASTIdentifier::getChild() const {
    return Child;
}

std::string ASTIdentifier::str() const {
    return Logger("ASTIdentifier").
            Super(ASTBase::str()).
            Attr("Name", Name).
            Attr("Child", Child).
            End();
}

