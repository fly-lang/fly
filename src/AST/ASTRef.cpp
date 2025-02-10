//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentifier.cpp - AST Identifier implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTRef.h"

using namespace fly;


ASTRef::ASTRef(const SourceLocation &Loc, llvm::StringRef Name, ASTRefKind Kind) :
        ASTBase(Loc, ASTKind::AST_REF), Name(Name), RefKind(Kind), Resolved(false) {
    FullName = Name.data();
}


ASTRef::~ASTRef() {
    delete Parent;
}

llvm::StringRef ASTRef::getName() const {
    return Name;
}

std::string ASTRef::getFullName() const {
    return FullName;
}

bool ASTRef::isResolved() const {
    return Resolved;
}

bool ASTRef::isType() const {
    return RefKind == ASTRefKind::REF_TYPE;
}

bool ASTRef::isCall() const {
    return RefKind == ASTRefKind::REF_CALL;
}

bool ASTRef::isVarRef() const {
    return RefKind == ASTRefKind::REF_VAR;
}

ASTRefKind ASTRef::getRefKind() const {
    return RefKind;
}

void ASTRef::AddChild(ASTRef *Identifier) {
    Child = Identifier;
    Child->Parent = this;
    Child->FullName = FullName.append(".").append(Child->Name.data());
}

ASTRef *ASTRef::getParent() const {
    return Parent;
}

ASTRef *ASTRef::getChild() const {
    return Child;
}

std::string ASTRef::str() const {
    return Logger("ASTIdentifier").
            Super(ASTBase::str()).
            Attr("Name", Name).
            Attr("Child", Child).
            End();
}

