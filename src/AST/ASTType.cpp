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

ASTType::ASTType(const SourceLocation &Loc, TypeKind Kind) : Loc(Loc), Kind(Kind) {

}

const SourceLocation &ASTType::getLocation() const  {
    return Loc;
}

const bool ASTType::isBool() const {
    return Kind == TYPE_BOOL;
}

const bool ASTType::isNumber() const {
    return Kind > 1 && Kind < 11;
}

const bool ASTType::isInteger() const {
    return Kind > 1 && Kind < 9;
}

const bool ASTType::isFloatingPoint() const {
    return Kind == TYPE_FLOAT || Kind == TYPE_DOUBLE;
}

const bool ASTType::isClass() const {
    return Kind == TYPE_CLASS;
}

const bool ASTType::isArray() const {
    return Kind == TYPE_ARRAY;
}

const TypeKind &ASTType::getKind() const  {
    return Kind;
}

bool ASTType::equals(ASTType *Ty) const {
    return this->getKind() == Ty->getKind();
}

ASTVoidType::ASTVoidType(const SourceLocation &Loc) : ASTType(Loc, TypeKind::TYPE_VOID) {

}

ASTBoolType::ASTBoolType(const SourceLocation &Loc) : ASTType(Loc, TypeKind::TYPE_BOOL) {

}

ASTByteType::ASTByteType(const SourceLocation &Loc) : ASTType(Loc, TypeKind::TYPE_BYTE) {

}

ASTUShortType::ASTUShortType(const SourceLocation &Loc) : ASTType(Loc, TypeKind::TYPE_USHORT) {

}

ASTShortType::ASTShortType(const SourceLocation &Loc) : ASTType(Loc, TypeKind::TYPE_SHORT) {

}

ASTUIntType::ASTUIntType(const SourceLocation &Loc)  : ASTType(Loc, TypeKind::TYPE_UINT) {

}

ASTIntType::ASTIntType(const SourceLocation &Loc)  : ASTType(Loc, TypeKind::TYPE_INT) {

}

ASTULongType::ASTULongType(const SourceLocation &Loc)  : ASTType(Loc, TypeKind::TYPE_ULONG) {

}

ASTLongType::ASTLongType(const SourceLocation &Loc)  : ASTType(Loc, TypeKind::TYPE_LONG) {

}

ASTFloatType::ASTFloatType(const SourceLocation &Loc) : ASTType(Loc, TypeKind::TYPE_FLOAT) {

}

ASTDoubleType::ASTDoubleType(const SourceLocation &Loc) : ASTType(Loc, TypeKind::TYPE_DOUBLE) {

}

ASTArrayType::ASTArrayType(const SourceLocation &Loc, ASTType *Type, ASTExpr *Size) :
        ASTType(Loc, TypeKind::TYPE_ARRAY), Type(Type), Size(Size) {

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
    ASTType(Loc, TypeKind::TYPE_CLASS), Name(Name) {

}

const std::string &ASTClassType::getName() const {
    return Name;
}

bool ASTClassType::operator==(const ASTClassType &Ty) const {
    return getKind() == Ty.getKind() && Name == Ty.Name;
}

const std::string &ASTClassType::getNameSpace() const {
    return NameSpace;
}

ASTClass *ASTClassType::getDef() const {
    return Def;
}

std::string ASTClassType::str() const {
    return "Name=" + Name + ", NameSpace=" + NameSpace;
}
