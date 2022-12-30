//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTType.cpp - Type implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTType.h"
#include "AST/ASTExpr.h"
#include "AST/ASTIdentifier.h"

using namespace fly;

ASTType::ASTType(const SourceLocation &Loc, ASTTypeKind Kind, ASTMacroTypeKind MacroKind) :
    Loc(Loc), Kind(Kind), MacroKind(MacroKind) {

}

const SourceLocation &ASTType::getLocation() const  {
    return Loc;
}

const ASTTypeKind &ASTType::getKind() const  {
    return Kind;
}

const ASTMacroTypeKind &ASTType::getMacroKind() const  {
    return MacroKind;
}

const bool ASTType::isBool() const {
    return MacroKind == ASTMacroTypeKind::MACRO_TYPE_BOOL;
}

const bool ASTType::isFloatingPoint() const {
    return MacroKind == ASTMacroTypeKind::MACRO_TYPE_FLOATING_POINT;
}

const bool ASTType::isInteger() const {
    return MacroKind == ASTMacroTypeKind::MACRO_TYPE_INTEGER;
}

const bool ASTType::isSignedInteger() const {
    return isInteger() && isSigned();
}

const bool ASTType::isUnsignedInteger() const {
    return isInteger() && isUnsigned();
}

const bool ASTType::isUnsigned() const {
    return (int) Kind % 2 == 0;
}

const bool ASTType::isSigned() const {
    return isUnsigned();
}

const bool ASTType::isNumber() const {
    return MacroKind == ASTMacroTypeKind::MACRO_TYPE_INTEGER || MacroKind == ASTMacroTypeKind::MACRO_TYPE_FLOATING_POINT;
}

const bool ASTType::isArray() const {
    return MacroKind == ASTMacroTypeKind::MACRO_TYPE_ARRAY;
}

const bool ASTType::isClass() const {
    return MacroKind == ASTMacroTypeKind::MACRO_TYPE_CLASS;
}

const bool ASTType::isVoid() const {
    return MacroKind == ASTMacroTypeKind::MACRO_TYPE_VOID;
}

const std::string ASTType::printMacroType() {
    return printMacroType(MacroKind);
}

const std::string ASTType::printMacroType(const ASTMacroTypeKind Kind) {
    switch (Kind) {

        case ASTMacroTypeKind::MACRO_TYPE_VOID:
            return "Void";
        case ASTMacroTypeKind::MACRO_TYPE_BOOL:
            return "Boolean";
        case ASTMacroTypeKind::MACRO_TYPE_INTEGER:
            return "Integer";
        case ASTMacroTypeKind::MACRO_TYPE_FLOATING_POINT:
            return "Floating Point";
        case ASTMacroTypeKind::MACRO_TYPE_ARRAY:
            return "Array";
        case ASTMacroTypeKind::MACRO_TYPE_CLASS:
            return "Class";
    }
}

std::string ASTType::str() const {
    return Logger("ASTType").
           Attr("Location", Loc).
           Attr("Kind", (uint64_t) Kind).
           Attr("MacroKind", (uint64_t) MacroKind).
           End();
}

ASTVoidType::ASTVoidType(const SourceLocation &Loc) : ASTType(Loc, ASTTypeKind::TYPE_VOID, ASTMacroTypeKind::MACRO_TYPE_VOID) {

}

const std::string ASTVoidType::print() const {
    return "";
}

std::string ASTVoidType::str() const {
    return Logger("ASTVoidType").
           Super(ASTType::str()).
           End();
}

ASTBoolType::ASTBoolType(const SourceLocation &Loc) : ASTType(Loc, ASTTypeKind::TYPE_BOOL, ASTMacroTypeKind::MACRO_TYPE_BOOL) {

}

const std::string ASTBoolType::print() const {
    return "bool";
}

std::string ASTBoolType::str() const {
    return Logger("ASTBoolType").
           Super(ASTType::str()).
           End();
}

ASTByteType::ASTByteType(const SourceLocation &Loc) : ASTType(Loc, ASTTypeKind::TYPE_BYTE, ASTMacroTypeKind::MACRO_TYPE_INTEGER) {

}

const std::string ASTByteType::print() const {
    return "byte";
}

std::string ASTByteType::str() const {
    return Logger("ASTBypeType").
           Super(ASTType::str()).
           End();
}

ASTUShortType::ASTUShortType(const SourceLocation &Loc) : ASTType(Loc, ASTTypeKind::TYPE_USHORT, ASTMacroTypeKind::MACRO_TYPE_INTEGER) {

}

const std::string ASTUShortType::print() const {
    return "ushort";
}

std::string ASTUShortType::str() const {
    return Logger("ASTUShortType").
           Super(ASTType::str()).
           End();
}

ASTShortType::ASTShortType(const SourceLocation &Loc) : ASTType(Loc, ASTTypeKind::TYPE_SHORT, ASTMacroTypeKind::MACRO_TYPE_INTEGER) {

}

const std::string ASTShortType::print() const {
    return "short";
}

std::string ASTShortType::str() const {
    return Logger("ASTShortType").
           Super(ASTType::str()).
           End();
}

ASTUIntType::ASTUIntType(const SourceLocation &Loc)  : ASTType(Loc, ASTTypeKind::TYPE_UINT, ASTMacroTypeKind::MACRO_TYPE_INTEGER) {

}

const std::string ASTUIntType::print() const {
    return "uint";
}

std::string ASTUIntType::str() const {
    return Logger("ASTUIntType").
           Super(ASTType::str()).
           End();
}

ASTIntType::ASTIntType(const SourceLocation &Loc)  : ASTType(Loc, ASTTypeKind::TYPE_INT, ASTMacroTypeKind::MACRO_TYPE_INTEGER) {

}

const std::string ASTIntType::print() const {
    return "int";
}

std::string ASTIntType::str() const {
    return Logger("ASTIntType").
           Super(ASTType::str()).
           End();
}

ASTULongType::ASTULongType(const SourceLocation &Loc)  : ASTType(Loc, ASTTypeKind::TYPE_ULONG, ASTMacroTypeKind::MACRO_TYPE_INTEGER) {

}

const std::string ASTULongType::print() const {
    return "ulong";
}

std::string ASTULongType::str() const {
    return Logger("ASTULongType").
           Super(ASTType::str()).
           End();
}

ASTLongType::ASTLongType(const SourceLocation &Loc)  : ASTType(Loc, ASTTypeKind::TYPE_LONG, ASTMacroTypeKind::MACRO_TYPE_INTEGER) {

}

const std::string ASTLongType::print() const {
    return "long";
}

std::string ASTLongType::str() const {
    return Logger("ASTLongType").
           Super(ASTType::str()).
           End();
}

ASTFloatType::ASTFloatType(const SourceLocation &Loc) : ASTType(Loc, ASTTypeKind::TYPE_FLOAT, ASTMacroTypeKind::MACRO_TYPE_FLOATING_POINT) {

}

const std::string ASTFloatType::print() const {
    return "float";
}

std::string ASTFloatType::str() const {
    return Logger("ASTFloatType").
           Super(ASTType::str()).
           End();
}

ASTDoubleType::ASTDoubleType(const SourceLocation &Loc) : ASTType(Loc, ASTTypeKind::TYPE_DOUBLE, ASTMacroTypeKind::MACRO_TYPE_FLOATING_POINT) {

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
        ASTType(Loc, ASTTypeKind::TYPE_ARRAY, ASTMacroTypeKind::MACRO_TYPE_ARRAY), Type(Type), Size(Size) {

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

ASTClassType::ASTClassType(const SourceLocation &Loc, llvm::StringRef NameSpace, llvm::StringRef Name, ASTClassType *Parent) :
        ASTType(Loc, ASTTypeKind::TYPE_CLASS, ASTMacroTypeKind::MACRO_TYPE_CLASS),
        Name(Name), NameSpace(NameSpace), Parent(Parent) {

}

ASTClassType *ASTClassType::getParent() const {
    return Parent;
}

llvm::StringRef ASTClassType::getName() const {
    return Name;
}

bool ASTClassType::operator==(const ASTClassType &Ty) const {
    return getKind() == Ty.getKind() && Name == Ty.Name;
}

llvm::StringRef ASTClassType::getNameSpace() const {
    return NameSpace;
}

ASTClass *ASTClassType::getDef() const {
    return Def;
}

const std::string ASTClassType::print() const {
    return std::string(NameSpace) + "." + std::string(Name);
}

std::string ASTClassType::str() const {
    return Logger("ASTClassType").
           Super(ASTType::str()).
           Attr("Name", Name).
           Attr("NameSpace", NameSpace).
           Attr("Def", (Debuggable *) Def).
           End();
}
