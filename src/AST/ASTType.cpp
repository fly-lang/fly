//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTType.cpp - AST Type implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTType.h"

using namespace fly;

ASTType::ASTType(const SourceLocation &Loc, ASTTypeKind MacroKind) :
        ASTBase(Loc), Kind(MacroKind) {

}

const ASTTypeKind &ASTType::getKind() const  {
    return Kind;
}

bool ASTType::isBool() const {
    return Kind == ASTTypeKind::TYPE_BOOL;
}

bool ASTType::isFloatingPoint() const {
    return Kind == ASTTypeKind::TYPE_FLOATING_POINT;
}

bool ASTType::isInteger() const {
    return Kind == ASTTypeKind::TYPE_INTEGER;
}

bool ASTType::isArray() const {
    return Kind == ASTTypeKind::TYPE_ARRAY;
}

bool ASTType::isString() const {
    return Kind == ASTTypeKind::TYPE_STRING;
}

bool ASTType::isIdentity() const {
    return Kind == ASTTypeKind::TYPE_IDENTITY;
}

bool ASTType::isError() const {
    return Kind == ASTTypeKind::TYPE_ERROR;
}

bool ASTType::isVoid() const {
    return Kind == ASTTypeKind::TYPE_VOID;
}

std::string ASTType::printType() {
    return printType(Kind);
}

std::string ASTType::printType(const ASTTypeKind Kind) {
    switch (Kind) {

        case ASTTypeKind::TYPE_VOID:
            return "Void";
        case ASTTypeKind::TYPE_BOOL:
            return "Boolean";
        case ASTTypeKind::TYPE_INTEGER:
            return "Integer";
        case ASTTypeKind::TYPE_FLOATING_POINT:
            return "Floating Point";
        case ASTTypeKind::TYPE_ARRAY:
            return "Array";
        case ASTTypeKind::TYPE_IDENTITY:
            return "Identity";
        case ASTTypeKind::TYPE_STRING:
            return "String";
        case ASTTypeKind::TYPE_ERROR:
            return "Error";
    }

    assert(false && "Unknown Type");
}

std::string ASTType::str() const {
    return Logger("ASTType").
            Super(ASTBase::str()).
            Attr("Kind", (uint64_t) Kind).
            End();
}

ASTVoidType::ASTVoidType(const SourceLocation &Loc) : ASTType(Loc, ASTTypeKind::TYPE_VOID) {

}

std::string ASTVoidType::print() const {
    return "void";
}

std::string ASTVoidType::str() const {
    return Logger("ASTVoidType").
           Super(ASTType::str()).
           End();
}

ASTBoolType::ASTBoolType(const SourceLocation &Loc) : ASTType(Loc, ASTTypeKind::TYPE_BOOL) {

}

std::string ASTBoolType::print() const {
    return "bool";
}

std::string ASTBoolType::str() const {
    return Logger("ASTBoolType").
           Super(ASTType::str()).
           End();
}

ASTIntegerType::ASTIntegerType(const SourceLocation &Loc, ASTIntegerTypeKind Kind) :
    ASTType(Loc, ASTTypeKind::TYPE_INTEGER), Kind(Kind) {

}

ASTIntegerTypeKind ASTIntegerType::getIntegerKind() const {
    return Kind;
}

bool ASTIntegerType::isUnsigned() const {
    return (int) Kind % 2 == 0;
}

bool ASTIntegerType::isSigned() const {
    return !isUnsigned();
}

uint32_t ASTIntegerType::getSize() {
    switch (Kind) {

        case ASTIntegerTypeKind::TYPE_BYTE:
            return 8;
        case ASTIntegerTypeKind::TYPE_SHORT:
        case ASTIntegerTypeKind::TYPE_USHORT:
            return 16;
        case ASTIntegerTypeKind::TYPE_INT:
        case ASTIntegerTypeKind::TYPE_UINT:
            return 32;
        case ASTIntegerTypeKind::TYPE_ULONG:
        case ASTIntegerTypeKind::TYPE_LONG:
            return 64;
    }

    assert(false && "Unknown Type Kind");
}

ASTFloatingPointType::ASTFloatingPointType(const SourceLocation &Loc, ASTFloatingPointTypeKind Kind) :
        ASTType(Loc, ASTTypeKind::TYPE_FLOATING_POINT), Kind(Kind) {

}

ASTFloatingPointTypeKind ASTFloatingPointType::getFloatingPointKind() const {
    return Kind;
}

uint32_t ASTFloatingPointType::getSize() {
    switch (Kind) {

        case ASTFloatingPointTypeKind::TYPE_FLOAT:
            return 32;
        case ASTFloatingPointTypeKind::TYPE_DOUBLE:
            return 64;
    }

    assert(false && "Unknown Type Kind");
}

ASTByteType::ASTByteType(const SourceLocation &Loc) : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_BYTE) {

}

std::string ASTByteType::print() const {
    return "byte";
}

std::string ASTByteType::str() const {
    return Logger("ASTBypeType").
           Super(ASTType::str()).
           End();
}

ASTUShortType::ASTUShortType(const SourceLocation &Loc) : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_USHORT) {

}

std::string ASTUShortType::print() const {
    return "ushort";
}

std::string ASTUShortType::str() const {
    return Logger("ASTUShortType").
           Super(ASTType::str()).
           End();
}

ASTShortType::ASTShortType(const SourceLocation &Loc) : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_SHORT) {

}

std::string ASTShortType::print() const {
    return "short";
}

std::string ASTShortType::str() const {
    return Logger("ASTShortType").
           Super(ASTType::str()).
           End();
}

ASTUIntType::ASTUIntType(const SourceLocation &Loc)  : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_UINT) {

}

std::string ASTUIntType::print() const {
    return "uint";
}

std::string ASTUIntType::str() const {
    return Logger("ASTUIntType").
           Super(ASTType::str()).
           End();
}

ASTIntType::ASTIntType(const SourceLocation &Loc)  : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_INT) {

}

std::string ASTIntType::print() const {
    return "int";
}

std::string ASTIntType::str() const {
    return Logger("ASTIntType").
           Super(ASTType::str()).
           End();
}

ASTULongType::ASTULongType(const SourceLocation &Loc)  : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_ULONG) {

}

std::string ASTULongType::print() const {
    return "ulong";
}

std::string ASTULongType::str() const {
    return Logger("ASTULongType").
           Super(ASTType::str()).
           End();
}

ASTLongType::ASTLongType(const SourceLocation &Loc)  : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_LONG) {

}

std::string ASTLongType::print() const {
    return "long";
}

std::string ASTLongType::str() const {
    return Logger("ASTLongType").
           Super(ASTType::str()).
           End();
}

ASTFloatType::ASTFloatType(const SourceLocation &Loc) : ASTFloatingPointType(Loc, ASTFloatingPointTypeKind::TYPE_FLOAT) {

}

std::string ASTFloatType::print() const {
    return "float";
}

std::string ASTFloatType::str() const {
    return Logger("ASTFloatType").
           Super(ASTType::str()).
           End();
}

ASTDoubleType::ASTDoubleType(const SourceLocation &Loc) : ASTFloatingPointType(Loc, ASTFloatingPointTypeKind::TYPE_DOUBLE) {

}

std::string ASTDoubleType::print() const {
    return "double";
}

std::string ASTDoubleType::str() const {
    return Logger("ASTDoubleType").
           Super(ASTType::str()).
           End();
}

ASTArrayType::ASTArrayType(const SourceLocation &Loc, ASTType *Type, ASTExpr *Size) :
        ASTType(Loc, ASTTypeKind::TYPE_ARRAY), Type(Type), Size(Size) {

}

ASTExpr *ASTArrayType::getSize() const {
    return Size;
}

ASTType *ASTArrayType::getType() const {
    return Type;
}

std::string ASTArrayType::str() const {
    return Logger("ASTArrayType").
           Super(ASTType::str()).
           Attr("Size", Size).
           Attr("Type", Type).
           End();
}

std::string ASTArrayType::print() const {
    return Type->print() + "[]";
}

ASTStringType::ASTStringType(const SourceLocation &Loc) :
        ASTType(Loc, ASTTypeKind::TYPE_STRING) {

}

std::string ASTStringType::print() const {
    return "string";
}

std::string ASTStringType::str() const {
    return Logger("ASTStringType").
            Super(ASTType::str()).
            End();
}

ASTErrorType::ASTErrorType(const SourceLocation &Loc) :
        ASTType(Loc, ASTTypeKind::TYPE_ERROR) {

}

std::string ASTErrorType::print() const {
    return "error";
}

std::string ASTErrorType::str() const {
    return Logger("ASTErrorType").
            Super(ASTType::str()).
            End();
}