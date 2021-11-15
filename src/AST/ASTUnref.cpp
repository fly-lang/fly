//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTValue.h - Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTUnref.h"
#include "AST/ASTNode.h"
#include "AST/ASTVar.h"
#include "AST/ASTFunc.h"

using namespace fly;

ASTUnref::ASTUnref(ASTNode *Node) : Node(Node) {

}

ASTNode *ASTUnref::getNode() {
    return Node;
}

ASTUnrefGlobalVar::ASTUnrefGlobalVar(ASTNode *Node, ASTVarRef &VarRef) : ASTUnref(Node), VarRef(VarRef) {

}

ASTVarRef &ASTUnrefGlobalVar::getVarRef() {
    return VarRef;
}

std::string ASTUnrefGlobalVar::str() const {
    return "Node=" + Node->str() + ", " +
           "VarRef=" + VarRef.str();
}

ASTUnrefCall::ASTUnrefCall(ASTNode *Node, ASTFuncCall *Call) : ASTUnref(Node), Call(Call) {

}

ASTFuncCall *ASTUnrefCall::getCall() {
    return Call;
}

std::string ASTUnrefCall::str() const {
    return "Node=" + Node->str() + ", " +
           "Call=" + Call->str();
}
