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
#include "AST/ASTVarRef.h"
#include "AST/ASTCall.h"

using namespace fly;

ASTUnref::ASTUnref(ASTNode *Node) : Node(Node) {

}

ASTNode *ASTUnref::getNode() {
    return Node;
}

std::string ASTUnref::str() const {
    return Logger("ASTUnref").End();
}

ASTUnrefGlobalVar::ASTUnrefGlobalVar(ASTNode *Node, ASTVarRef &VarRef) : ASTUnref(Node), VarRef(VarRef) {

}

ASTVarRef &ASTUnrefGlobalVar::getVarRef() {
    return VarRef;
}

std::string ASTUnrefGlobalVar::str() const {
    return Logger("ASTUnrefGlobalVar").
            Super(ASTUnref::str()).
            Attr("VarRef", &VarRef).
            End();
}

ASTUnrefFunctionCall::ASTUnrefFunctionCall(ASTNode *Node, ASTCall *Call) : ASTUnref(Node), Call(Call) {

}

ASTCall *ASTUnrefFunctionCall::getCall() {
    return Call;
}

std::string ASTUnrefFunctionCall::str() const {
    return Logger("ASTUnrefFunctionCall").
            Attr("Call", Call).
            End();
}
