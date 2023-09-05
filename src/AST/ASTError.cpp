//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTError.cpp - Error
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTError.h"

using namespace fly;

ASTError::ASTError(const SourceLocation &Loc, uint32_t Code) :
        Loc(Loc), ErrorKind(ASTErrorKind::ERR_CODE), Enabled(Code != 0), Code(Code), Class(nullptr) {

}

ASTError::ASTError(const SourceLocation &Loc, llvm::StringRef Message) :
        Loc(Loc), ErrorKind(ASTErrorKind::ERR_MESSAGE), Enabled(!Message.empty()), Code(0), Message(Message), Class(nullptr) {

}

ASTError::ASTError(const SourceLocation &Loc, ASTClassType *Class) :
        Loc(Loc), ErrorKind(ASTErrorKind::ERR_CLASS), Enabled(Class != nullptr), Code(0), Class(Class) {

}

bool ASTError::isEnabled() const {
    return Enabled;
}

uint32_t ASTError::getCode() const {
    return Code;
}

llvm::StringRef ASTError::getMessage() const {
    return Message;
}

ASTErrorKind ASTError::getErrorKind() const {
    return ErrorKind;
}

const SourceLocation &ASTError::getLocation() const {
    return Loc;
}

std::string ASTError::print() const {
    return "error";
}

std::string ASTError::str() const {
    return Logger("ASTError").
            Attr("Code", (uint64_t) Code).
            Attr("Message", Message).
            Attr("FailKind", (uint64_t) ErrorKind).
            End();
}
