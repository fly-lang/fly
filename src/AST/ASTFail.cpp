//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFail.cpp - Fail
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFail.h"

using namespace fly;

ASTFail::ASTFail(ASTStmt *Parent, const SourceLocation &Loc, uint32_t Code) :
        ASTStmt(Parent, Loc, ASTStmtKind::STMT_FAIL),
        FailKind(ASTFailKind::ERR_NUMBER), Code(Code), Class(nullptr) {

}

ASTFail::ASTFail(ASTStmt *Parent, const SourceLocation &Loc, llvm::StringRef Message) :
        ASTStmt(Parent, Loc, ASTStmtKind::STMT_FAIL),
        FailKind(ASTFailKind::ERR_MESSAGE), Code(0), Message(Message), Class(nullptr) {

}

ASTFail::ASTFail(ASTStmt *Parent, const SourceLocation &Loc, ASTClassType *Class) :
        ASTStmt(Parent, Loc, ASTStmtKind::STMT_FAIL),
        FailKind(ASTFailKind::ERR_CLASS), Code(0), Class(Class) {

}

ASTFail::~ASTFail() {

}

uint32_t ASTFail::getCode() const {
    return Code;
}

llvm::StringRef ASTFail::getMessage() const {
    return Message;
}

ASTFailKind ASTFail::getFailKind() const {
    return FailKind;
}

std::string ASTFail::print() const {
    return "error";
}

std::string ASTFail::str() const {
    return Logger("ASTFail").
            Attr("Code", (uint64_t) Code).
            Attr("Message", Message).
            Attr("FailKind", (uint64_t) FailKind).
            End();
}
