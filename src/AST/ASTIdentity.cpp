//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentity.cpp - Type implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTIdentity.h"

using namespace fly;

ASTIdentity::ASTIdentity(ASTNode *Node, ASTTopDefKind TopDefKind, ASTScopes *Scopes,const SourceLocation &Loc,
                         llvm::StringRef Name) : ASTTopDef(Node, TopDefKind, Scopes), Location(Loc), Name(Name) {

}

llvm::StringRef ASTIdentity::getName() const {
    return Name;
}

const SourceLocation &ASTIdentity::getLocation() const {
    return Location;
}