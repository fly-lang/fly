//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentifier.cpp - AST Identifier implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTIdentifier.h"
#include "Basic/Logger.h"

using namespace fly;


ASTIdentifier::ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTIdentifierKind Kind) :
        ASTNode(Loc, ASTKind::AST_REF), Name(Name), RefKind(Kind), Visited(false) {
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

SemaResult *ASTIdentifier::getSema() const {
	return Sema;
}

void ASTIdentifier::setSema(SemaResult *Sema) {
    this->Sema = Sema;
}

bool ASTIdentifier::isVisited() const {
    return Visited;
}

void ASTIdentifier::setVisited(bool Visited) {
	this->Visited = Visited;
}

bool ASTIdentifier::isCall() const {
    return RefKind == ASTIdentifierKind::CALL;
}

bool ASTIdentifier::isVarRef() const {
    return RefKind == ASTIdentifierKind::VAR;
}

ASTIdentifierKind ASTIdentifier::getRefKind() const {
    return RefKind;
}

void ASTIdentifier::setChild(ASTIdentifier *Identifier) {
	this->Child = Identifier;
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
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
            Attr("Name", Name).
            Attr("Child", Child).
            End();
}

