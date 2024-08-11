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

ASTIdentifier::ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name) :
        ASTBase(Loc), Name(Name), Kind(ASTIdentifierKind::REF_UNDEFINED) {
    FullName = Name.data();
}

ASTIdentifier::ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTIdentifierKind Kind) :
        ASTBase(Loc), Name(Name), Kind(Kind), Resolved(true) {
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

bool ASTIdentifier::isNameSpace() const {
    return Kind == ASTIdentifierKind::REF_NAMESPACE;
}

bool ASTIdentifier::isType() const {
    return Kind == ASTIdentifierKind::REF_TYPE;
}

bool ASTIdentifier::isCall() const {
    return Kind == ASTIdentifierKind::REF_CALL;
}

bool ASTIdentifier::isVarRef() const {
    return Kind == ASTIdentifierKind::REF_VAR;
}

ASTIdentifierKind ASTIdentifier::getIdKind() const {
    return Kind;
}

ASTIdentifier *ASTIdentifier::AddChild(ASTIdentifier *Identifier) {
    Child = Identifier;
    Child->Parent = this;
    FullName.append(".").append(Name.data());
    return Child;
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

