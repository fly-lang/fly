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

using namespace fly;

ASTType::ASTType(const SourceLocation &Loc, TypeKind Kind, MacroTypeKind MacroKind) :
    Loc(Loc), Kind(Kind), MacroKind(MacroKind) {

}

const SourceLocation &ASTType::getLocation() const  {
    return Loc;
}

const TypeKind &ASTType::getKind() const  {
    return Kind;
}

const MacroTypeKind &ASTType::getMacroKind() const  {
    return MacroKind;
}

const bool ASTType::isBool() const {
    return MacroKind == MACRO_TYPE_BOOL;
}

const bool ASTType::isNumber() const {
    return MacroKind == MACRO_TYPE_INTEGER || MacroKind == MACRO_TYPE_FLOATING_POINT;
}

const bool ASTType::isClass() const {
    return MacroKind == MACRO_TYPE_CLASS;
}

const bool ASTType::isArray() const {
    return MacroKind == MACRO_TYPE_ARRAY;
}

const std::string ASTType::printMacroType() {
    return printMacroType(MacroKind);
}

const std::string ASTType::printMacroType(const MacroTypeKind Kind) {
    switch (Kind) {

        case MACRO_TYPE_VOID:
            return "Void";
        case MACRO_TYPE_BOOL:
            return "Boolean";
        case MACRO_TYPE_INTEGER:
            return "Integer";
        case MACRO_TYPE_FLOATING_POINT:
            return "Floating Point";
        case MACRO_TYPE_ARRAY:
            return "Array";
        case MACRO_TYPE_CLASS:
            return "Class";
    }
}

ASTVoidType::ASTVoidType(const SourceLocation &Loc) : ASTType(Loc, TYPE_VOID, MACRO_TYPE_VOID) {

}

ASTBoolType::ASTBoolType(const SourceLocation &Loc) : ASTType(Loc, TYPE_BOOL, MACRO_TYPE_BOOL) {

}

ASTByteType::ASTByteType(const SourceLocation &Loc) : ASTType(Loc, TYPE_BYTE, MACRO_TYPE_INTEGER) {

}

ASTUShortType::ASTUShortType(const SourceLocation &Loc) : ASTType(Loc, TYPE_USHORT, MACRO_TYPE_INTEGER) {

}

ASTShortType::ASTShortType(const SourceLocation &Loc) : ASTType(Loc, TYPE_SHORT, MACRO_TYPE_INTEGER) {

}

ASTUIntType::ASTUIntType(const SourceLocation &Loc)  : ASTType(Loc, TYPE_UINT, MACRO_TYPE_INTEGER) {

}

ASTIntType::ASTIntType(const SourceLocation &Loc)  : ASTType(Loc, TYPE_INT, MACRO_TYPE_INTEGER) {

}

ASTULongType::ASTULongType(const SourceLocation &Loc)  : ASTType(Loc, TYPE_ULONG, MACRO_TYPE_INTEGER) {

}

ASTLongType::ASTLongType(const SourceLocation &Loc)  : ASTType(Loc, TYPE_LONG, MACRO_TYPE_INTEGER) {

}

ASTFloatType::ASTFloatType(const SourceLocation &Loc) : ASTType(Loc, TYPE_FLOAT, MACRO_TYPE_FLOATING_POINT) {

}

ASTDoubleType::ASTDoubleType(const SourceLocation &Loc) : ASTType(Loc, TYPE_DOUBLE, MACRO_TYPE_FLOATING_POINT) {

}

ASTArrayType::ASTArrayType(const SourceLocation &Loc, ASTType *Type, ASTExpr *Size) :
        ASTType(Loc, TYPE_ARRAY, MACRO_TYPE_ARRAY), Type(Type), Size(Size) {

}

ASTExpr *ASTArrayType::getSize() const {
    return Size;
}

ASTType *ASTArrayType::getType() const {
    return Type;
}

std::string ASTArrayType::str() const {
    return Type->str() + "[]";
}

ASTClassType::ASTClassType(const SourceLocation &Loc, std::string Name, std::string NameSpace) :
        ASTType(Loc, TYPE_CLASS, MACRO_TYPE_CLASS), Name(Name) {

}

const std::string ASTClassType::getName() const {
    return Name;
}

bool ASTClassType::operator==(const ASTClassType &Ty) const {
    return getKind() == Ty.getKind() && Name == Ty.Name;
}

const std::string ASTClassType::getNameSpace() const {
    return NameSpace;
}

ASTClass *ASTClassType::getDef() const {
    return Def;
}

std::string ASTClassType::str() const {
    return "Name=" + Name + ", NameSpace=" + NameSpace;
}
