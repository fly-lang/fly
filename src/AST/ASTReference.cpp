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

ASTReference::ASTReference(ASTIdentifier *Identifier, bool Call) : Identifier(Identifier), Call(Call) {

}

SourceLocation ASTReference::getLocation() const {
    return Identifier ? Identifier->getLocation() : SourceLocation();
}

llvm::StringRef ASTReference::getName() const {
    return Identifier ? Identifier->getName() : llvm::StringRef(); // TODO move into VAR and CALL
}

ASTIdentifier *ASTReference::getIdentifier() const {
    return Identifier;
}

ASTReference *ASTReference::getInstance() const {
    return Instance;
}

CodeGenInstance *ASTReference::getCodeGen() const {
    return CodeGen;
}

bool ASTReference::isCall() const {
    return Call;
}

void ASTReference::setCodeGen(CodeGenInstance *CGI) {
    this->CodeGen = CGI;
}
