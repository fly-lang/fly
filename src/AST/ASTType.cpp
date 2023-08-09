//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTType.cpp - Type implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTType.h"

using namespace fly;

ASTType::ASTType(const SourceLocation &Loc, ASTTypeKind MacroKind) :
        Loc(Loc), Kind(MacroKind) {

}

const SourceLocation &ASTType::getLocation() const  {
    return Loc;
}

const ASTTypeKind &ASTType::getIdentityKind() const  {
    return Kind;
}

const bool ASTType::isBool() const {
    return Kind == ASTTypeKind::TYPE_BOOL;
}

const bool ASTType::isFloatingPoint() const {
    return Kind == ASTTypeKind::TYPE_FLOATING_POINT;
}

const bool ASTType::isInteger() const {
    return Kind == ASTTypeKind::TYPE_INTEGER;
}

const bool ASTType::isArray() const {
    return Kind == ASTTypeKind::TYPE_ARRAY;
}

const bool ASTType::isIdentity() const {
    return Kind == ASTTypeKind::TYPE_IDENTITY;
}

const bool ASTType::isVoid() const {
    return Kind == ASTTypeKind::TYPE_VOID;
}

const std::string ASTType::printType() {
    return printType(Kind);
}

const std::string ASTType::printType(const ASTTypeKind Kind) {
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
    }
}

std::string ASTType::str() const {
    return Logger("ASTType").
           Attr("Location", Loc).
           Attr("Kind", (uint64_t) Kind).
           End();
}

ASTVoidType::ASTVoidType(const SourceLocation &Loc) : ASTType(Loc, ASTTypeKind::TYPE_VOID) {

}

const std::string ASTVoidType::print() const {
    return "";
}

std::string ASTVoidType::str() const {
    return Logger("ASTVoidType").
           Super(ASTType::str()).
           End();
}

ASTBoolType::ASTBoolType(const SourceLocation &Loc) : ASTType(Loc, ASTTypeKind::TYPE_BOOL) {

}

const std::string ASTBoolType::print() const {
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

const bool ASTIntegerType::isUnsigned() const {
    return (int) Kind % 2 == 0;
}

const bool ASTIntegerType::isSigned() const {
    return !isUnsigned();
}

const uint32_t ASTIntegerType::getSize() {
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
}

ASTFloatingPointType::ASTFloatingPointType(const SourceLocation &Loc, ASTFloatingPointTypeKind Kind) :
        ASTType(Loc, ASTTypeKind::TYPE_FLOATING_POINT), Kind(Kind) {

}

ASTFloatingPointTypeKind ASTFloatingPointType::getFloatingPointKind() const {
    return Kind;
}

const uint32_t ASTFloatingPointType::getSize() {
    switch (Kind) {

        case ASTFloatingPointTypeKind::TYPE_FLOAT:
            return 32;
        case ASTFloatingPointTypeKind::TYPE_DOUBLE:
            return 64;
    }
}

ASTByteType::ASTByteType(const SourceLocation &Loc) : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_BYTE) {

}

const std::string ASTByteType::print() const {
    return "byte";
}

std::string ASTByteType::str() const {
    return Logger("ASTBypeType").
           Super(ASTType::str()).
           End();
}

ASTUShortType::ASTUShortType(const SourceLocation &Loc) : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_USHORT) {

}

const std::string ASTUShortType::print() const {
    return "ushort";
}

std::string ASTUShortType::str() const {
    return Logger("ASTUShortType").
           Super(ASTType::str()).
           End();
}

ASTShortType::ASTShortType(const SourceLocation &Loc) : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_SHORT) {

}

const std::string ASTShortType::print() const {
    return "short";
}

std::string ASTShortType::str() const {
    return Logger("ASTShortType").
           Super(ASTType::str()).
           End();
}

ASTUIntType::ASTUIntType(const SourceLocation &Loc)  : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_UINT) {

}

const std::string ASTUIntType::print() const {
    return "uint";
}

std::string ASTUIntType::str() const {
    return Logger("ASTUIntType").
           Super(ASTType::str()).
           End();
}

ASTIntType::ASTIntType(const SourceLocation &Loc)  : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_INT) {

}

const std::string ASTIntType::print() const {
    return "int";
}

std::string ASTIntType::str() const {
    return Logger("ASTIntType").
           Super(ASTType::str()).
           End();
}

ASTULongType::ASTULongType(const SourceLocation &Loc)  : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_ULONG) {

}

const std::string ASTULongType::print() const {
    return "ulong";
}

std::string ASTULongType::str() const {
    return Logger("ASTULongType").
           Super(ASTType::str()).
           End();
}

ASTLongType::ASTLongType(const SourceLocation &Loc)  : ASTIntegerType(Loc, ASTIntegerTypeKind::TYPE_LONG) {

}

const std::string ASTLongType::print() const {
    return "long";
}

std::string ASTLongType::str() const {
    return Logger("ASTLongType").
           Super(ASTType::str()).
           End();
}

ASTFloatType::ASTFloatType(const SourceLocation &Loc) : ASTFloatingPointType(Loc, ASTFloatingPointTypeKind::TYPE_FLOAT) {

}

const std::string ASTFloatType::print() const {
    return "float";
}

std::string ASTFloatType::str() const {
    return Logger("ASTFloatType").
           Super(ASTType::str()).
           End();
}

ASTDoubleType::ASTDoubleType(const SourceLocation &Loc) : ASTFloatingPointType(Loc, ASTFloatingPointTypeKind::TYPE_DOUBLE) {

}

const std::string ASTDoubleType::print() const {
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

const std::string ASTArrayType::print() const {
    return Type->print() + "[]";
}
