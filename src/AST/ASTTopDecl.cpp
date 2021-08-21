//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTTopDecl.cpp - Top declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTTopDecl.h"
#include "AST/ASTNode.h"

using namespace fly;

ASTTopDecl::ASTTopDecl(ASTNode *Node, const SourceLocation &Loc) : Node(Node), Location(Loc), Visibility(V_DEFAULT) {

}

ASTNode *ASTTopDecl::getNode() {
    return Node;
}

ASTNameSpace *ASTTopDecl::getNameSpace() const {
    return Node->getNameSpace();
}

const SourceLocation &ASTTopDecl::getLocation() const {
    return Location;
}

VisibilityKind ASTTopDecl::getVisibility() const {
    return Visibility;
}

void ASTTopDecl::setVisibility(VisibilityKind V) {
    Visibility = V;
}
