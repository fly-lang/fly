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

ASTIdentifier::ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name) : Loc(Loc), Name(Name) {
    PrintName = Name.data();
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

ASTIdentifier *ASTIdentifier::AddChild(const SourceLocation &Loc, const llvm::StringRef Name) {
    Child = new ASTIdentifier(Loc, Name);
    Child->Parent = this;
    Child->Index = Index + 1;
    if (!Parent) {
        this->asRoot = true;
        Child->Root = this;
    }
    PrintName.append(".").append(Name.data());
    return Child;
}

uint32_t ASTIdentifier::getIndex() const {
    return Index;
}

ASTIdentifier *ASTIdentifier::getRoot() const {
    return Root;
}

bool ASTIdentifier::isRoot() const {
    return asRoot;
}

ASTIdentifier *ASTIdentifier::getParent() const {
    return Parent;
}

ASTIdentifier *ASTIdentifier::getChild() const {
    return Child;
}

bool ASTIdentifier::isCall() const {
    return RefIsCall;
}

ASTVarRef *ASTIdentifier::getVarRef() const {
    if (RefIsCall)
        return nullptr;
    return (ASTVarRef *) Reference;
}

ASTCall *ASTIdentifier::getCall() const {
    if (!RefIsCall)
        return nullptr;
    return (ASTCall *) Reference;
}

void ASTIdentifier::setCall(ASTCall *Call) {
    RefIsCall = true;
    this->Reference = (ASTReference *) Call;
}

std::string ASTIdentifier::print() const {
    return PrintName;
}

std::string ASTIdentifier::str() const {
    std::string StrName;
    return Logger("ASTIdentifier").
            Attr("Reference", Reference).
            Attr("Name", Name).
            Attr("Root", Root).
            Attr("Parent", Parent).
            Attr("Child", Child).
            End();
}
