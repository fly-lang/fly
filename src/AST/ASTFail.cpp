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

ASTFail::ASTFail(const SourceLocation &Loc, ASTErrorKind Kind) : Loc(Loc), Kind(Kind) {

}

ASTFail::ASTFail(const SourceLocation &Loc, ASTErrorKind Kind, llvm::StringRef Message) : Loc(Loc), Kind(Kind), Message(Message) {

}


ASTFail::~ASTFail() {

}

const SourceLocation &ASTFail::getLocation() const {
    return Loc;
}

llvm::StringRef ASTFail::getMessage() const {
    return Message;
}

ASTErrorKind ASTFail::getKind() const {
    return Kind;
}

std::string ASTFail::print() const {
    return "error";
}

std::string ASTFail::str() const {
    return Logger("ASTFail").
            Attr("Message", Message).
            Attr("ErrorKind", (uint64_t) Kind).
            End();
}
