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

ASTTopDecl::ASTTopDecl(const SourceLocation &Loc, ASTNode *Node, TopDeclKind Kind) : Location(Loc), Node(Node),
        Visibility(V_DEFAULT), Kind(Kind) {

}

TopDeclKind ASTTopDecl::getKind() const {
    assert(Kind != DECL_NONE && "Invalid Kind");
    return Kind;
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

std::string ASTTopDecl::str() const {
    return "Visibility=" + std::to_string(Visibility) +
            ", Kind=" + std::to_string(Kind);
}
