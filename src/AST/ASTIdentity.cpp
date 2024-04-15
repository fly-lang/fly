//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentity.cpp - Type implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTIdentity.h"
#include "AST/ASTNode.h"

using namespace fly;

ASTIdentity::ASTIdentity(ASTNode *Node, ASTTopDefKind TopDefKind, ASTScopes *Scopes,const SourceLocation &Loc,
                         llvm::StringRef Name) :
                         Node(Node), TopDefKind(TopDefKind), Scopes(Scopes), Location(Loc), Name(Name) {

}

llvm::StringRef ASTIdentity::getName() const {
    return Name;
}

const SourceLocation &ASTIdentity::getLocation() const {
    return Location;
}

ASTTopDefKind ASTIdentity::getTopDefKind() const {
    return TopDefKind;
}

ASTNode *ASTIdentity::getNode() const {
    return Node;
}

ASTNameSpace *ASTIdentity::getNameSpace() const {
    return Node->getNameSpace();
}

llvm::StringRef ASTIdentity::getComment() const {
    return Comment;
}

ASTScopes *ASTIdentity::getScopes() const {
    return Scopes;
}
