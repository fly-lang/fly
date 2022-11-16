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
    return Logger("ASTTopScopes").
           Attr("Visibility", (uint64_t) Visibility).
           Attr("Constant", Constant).
           End();
}

ASTTopDef::ASTTopDef(const SourceLocation &Loc, ASTNode *Node, ASTTopDefKind Kind, ASTTopScopes *Scopes) :
    Location(Loc), Node(Node), Scopes(Scopes), NameSpace(Node->getNameSpace()), Kind(Kind) {

}

const SourceLocation &ASTTopDef::getLocation() const {
    return Location;
}

ASTTopDefKind ASTTopDef::getKind() const {
    assert(Kind != ASTTopDefKind::DEF_NONE && "Invalid Kind");
    return Kind;
}

ASTNode *ASTTopDef::getNode() {
    return Node;
}

ASTNameSpace *ASTTopDef::getNameSpace() const {
    return NameSpace;
}

ASTTopScopes *ASTTopDef::getScopes() const {
    return Scopes;
}

llvm::StringRef ASTTopDef::getComment() const {
    return Comment;
}

std::string ASTTopDef::str() const {
    return Logger("ASTTopDef").
           Attr("Location", Location).
           Attr("Scopes", Scopes).
           Attr("Kind", (uint64_t) Kind).
           Attr("Comment", Comment).
           End();
}
