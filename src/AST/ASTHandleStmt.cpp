//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTHandleStmt.cpp - AST Handle Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTHandleStmt.h"

using namespace fly;

ASTHandleStmt::ASTHandleStmt(ASTStmt *Parent, const SourceLocation &Loc) :
        ASTStmt(Parent, Loc, ASTStmtKind::STMT_HANDLE) {

}

void ASTHandleStmt::setErrorHandlerRef(ASTVarRef *errorRef) {
    ErrorHandlerRef = errorRef;
}

ASTVarRef *ASTHandleStmt::getErrorHandlerRef() const {
    return ErrorHandlerRef;
}

ASTStmt *ASTHandleStmt::getHandle() const {
    return Handle;
}

void ASTHandleStmt::setHandle(ASTStmt *H) {
    Handle = H;
}

CodeGenHandle *ASTHandleStmt::getCodeGen() const {
    return CodeGen;
}

void ASTHandleStmt::setCodeGen(CodeGenHandle *codeGen) {
    CodeGen = codeGen;
}

std::string ASTHandleStmt::str() const {
    return Logger("ASTHandleBlock").
            Super(ASTStmt::str()).
            Attr("ErrorRef", ErrorHandlerRef).
            Attr("Handle", Handle).
            End();
}
