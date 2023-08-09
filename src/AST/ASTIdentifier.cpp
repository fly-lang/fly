//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentifier.cpp - Identifier implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTIdentifier.h"

using namespace fly;

ASTIdentifier::ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name) : Loc(Loc), Name(Name), Kind(ASTIdentifierKind::REF_UNDEF) {
    FullName = Name.data();
}

ASTIdentifier::ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTIdentifierKind Kind) : Loc(Loc), Name(Name), Kind(Kind) {
    FullName = Name.data();
}


ASTIdentifier::~ASTIdentifier() {
    delete Parent;
}

const SourceLocation &ASTIdentifier::getLocation() const {
    return Loc;
}

llvm::StringRef ASTIdentifier::getName() const {
    return Name;
}

std::string ASTIdentifier::getFullName() const {
    return FullName;
}

bool ASTIdentifier::isUndef() const {
    return Kind == ASTIdentifierKind::REF_UNDEF;
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

ASTIdentifierKind ASTIdentifier::getKind() const {
    return Kind;
}

ASTIdentifier *ASTIdentifier::AddChild(const SourceLocation &Loc, const llvm::StringRef Name) {
    Child = new ASTIdentifier(Loc, Name);
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

std::string ASTIdentifier::print() const {
    return FullName;
}

std::string ASTIdentifier::str() const {
    return Logger("ASTIdentifier").
            Attr("Name", Name).
            Attr("Child", Child).
            End();
}
