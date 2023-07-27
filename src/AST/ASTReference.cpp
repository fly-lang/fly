//===-------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTReference.cpp - Reference
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTReference.h"
#include "AST/ASTIdentifier.h"

using namespace fly;

ASTReference::ASTReference(ASTIdentifier *Identifier, ASTReferenceKind RefKind) : Identifier(Identifier), RefKind(RefKind) {

}

SourceLocation ASTReference::getLocation() const {
    return Identifier ? Identifier->getLocation() : SourceLocation();
}

llvm::StringRef ASTReference::getName() const {
    return Identifier ? Identifier->getName() : llvm::StringRef(); // TODO use VIRTUAL and move into VAR and CALL
}

ASTIdentifier *ASTReference::getIdentifier() const {
    return Identifier;
}

ASTReference *ASTReference::getInstance() const {
    return Instance;
}

ASTReferenceKind ASTReference::getRefKind() const {
    return RefKind;
}

bool ASTReference::isCall() const {
    RefKind == ASTReferenceKind::REF_CALL;
}
