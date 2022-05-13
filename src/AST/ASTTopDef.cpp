//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTTopDecl.cpp - Top declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTTopDef.h"
#include "AST/ASTNode.h"

using namespace fly;

ASTTopDef::ASTTopDef(const SourceLocation &Loc, ASTNode *Node, TopDeclKind Kind, VisibilityKind Visibility) :
    Location(Loc), Node(Node), Visibility(Visibility), NameSpace(Node->getNameSpace()), Kind(Kind) {

}

TopDeclKind ASTTopDef::getKind() const {
    assert(Kind != DECL_NONE && "Invalid Kind");
    return Kind;
}

ASTNode *ASTTopDef::getNode() {
    return Node;
}

ASTNameSpace *ASTTopDef::getNameSpace() const {
    return NameSpace;
}

const SourceLocation &ASTTopDef::getLocation() const {
    return Location;
}

VisibilityKind ASTTopDef::getVisibility() const {
    return Visibility;
}

void ASTTopDef::setVisibility(VisibilityKind V) {
    Visibility = V;
}

std::string ASTTopDef::str() const {
    return "Visibility=" + std::to_string(Visibility) +
            ", Kind=" + std::to_string(Kind);
}

const std::string &ASTTopDef::getComment() const {
    return Comment;
}
