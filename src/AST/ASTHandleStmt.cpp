//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTHandleStmt.cpp - AST Handle Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTHandleStmt.h"
#include "AST/ASTVarRef.h"

using namespace fly;

ASTHandleStmt::ASTHandleStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_HANDLE) {

}

ASTVarRef *ASTHandleStmt::getErrorHandlerRef() const {
    return ErrorHandlerRef;
}

void ASTHandleStmt::setErrorHandlerRef(ASTVarRef *ErrorHandler) {
    ErrorHandlerRef = ErrorHandler;
}

ASTBlockStmt* ASTHandleStmt::getHandle() const {
    return Handle;
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
            Attr("ErrorHandler", ErrorHandlerRef).
            End();
}
