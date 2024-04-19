//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTError.cpp - Error
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTError.h"
#include "AST/ASTEnum.h"
#include "AST/ASTClass.h"

using namespace fly;

ASTError::ASTError(const SourceLocation &Loc, uint32_t Code) :
        ASTBase(Loc), ErrorKind(ASTErrorKind::ERR_INT), Code(Code), Identity(nullptr) {

}

ASTError::ASTError(const SourceLocation &Loc, llvm::StringRef Message) :
        ASTBase(Loc), ErrorKind(ASTErrorKind::ERR_STRING), Code(0), Message(Message), Identity(nullptr) {

}

ASTError::ASTError(const SourceLocation &Loc, ASTEnum *Enum) :
        ASTBase(Loc), ErrorKind(ASTErrorKind::ERR_ENUM), Code(0), Identity(Enum) {

}

ASTError::ASTError(const SourceLocation &Loc, ASTClass *Class) :
        ASTBase(Loc), ErrorKind(ASTErrorKind::ERR_CLASS), Code(0), Identity(Class) {

}

uint32_t ASTError::getCode() const {
    return Code;
}

llvm::StringRef ASTError::getMessage() const {
    return Message;
}

ASTEnum* ASTError::getEnum() const {
    return (ASTEnum *) Identity;
}

ASTClass* ASTError::getClass() const {
    return (ASTClass *) Identity;
}

ASTErrorKind ASTError::getErrorKind() const {
    return ErrorKind;
}

std::string ASTError::print() const {
    return "error";
}

std::string ASTError::str() const {
    return Logger("ASTError").
            Super(ASTBase::str()).
            Attr("Code", (uint64_t) Code).
            Attr("Message", Message).
            Attr("FailKind", (uint64_t) ErrorKind).
            End();
}
