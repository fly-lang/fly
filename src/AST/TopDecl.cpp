//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/TopDecl.h - Top declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/TopDecl.h"
#include "AST/ASTNode.h"

using namespace fly;

TopDecl::TopDecl(ASTNode *Node, const SourceLocation &Loc) : Node(Node), Location(Loc), Visibility(V_DEFAULT) {

}

ASTNode *TopDecl::getNode() {
    return Node;
}

const ASTNameSpace &TopDecl::getNameSpace() const {
    return *Node->getNameSpace();
}

const SourceLocation &TopDecl::getLocation() const {
    return Location;
}

VisibilityKind TopDecl::getVisibility() const {
    return Visibility;
}

void TopDecl::setVisibility(VisibilityKind V) {
    Visibility = V;
}
