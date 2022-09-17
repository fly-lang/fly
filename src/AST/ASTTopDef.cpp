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

ASTTopScopes::ASTTopScopes(ASTVisibilityKind visibility, bool constant) : Visibility(visibility), Constant(constant) {

}

ASTVisibilityKind ASTTopScopes::getVisibility() const {
    return Visibility;
}

bool ASTTopScopes::isConstant() const {
    return Constant;
}

std::string ASTTopScopes::str() const {
    return "Visibility=" + std::to_string(Visibility) +
           ", Constant=" + std::to_string(Constant);
}

ASTTopDef::ASTTopDef(const SourceLocation &Loc, ASTNode *Node, ASTTopDefKind Kind, ASTTopScopes *Scopes) :
    Location(Loc), Node(Node), Scopes(Scopes), NameSpace(Node->getNameSpace()), Kind(Kind) {

}

ASTTopDefKind ASTTopDef::getKind() const {
    assert(Kind != DEF_NONE && "Invalid Kind");
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

ASTTopScopes *ASTTopDef::getScopes() const {
    return Scopes;
}

const std::string ASTTopDef::getComment() const {
    return Comment;
}

std::string ASTTopDef::str() const {
    return "ASTTopDef { Scopes=" + Scopes->str() +
            ", Kind=" + std::to_string(Kind) +
            ", Comment=" + Comment +
            " }";
}