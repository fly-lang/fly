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

ASTTopDef::ASTTopDef(ASTNode *Node, ASTTopDefKind Kind, ASTScopes *Scopes) :
        Node(Node), Scopes(Scopes), NameSpace(Node->getNameSpace()), Kind(Kind) {

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

ASTScopes *ASTTopDef::getScopes() const {
    return Scopes;
}

llvm::StringRef ASTTopDef::getComment() const {
    return Comment;
}

std::string ASTTopDef::str() const {
    return Logger("ASTTopDef").
           Attr("Scopes", Scopes).
           Attr("Kind", (uint64_t) Kind).
           Attr("Comment", Comment).
           End();
}
