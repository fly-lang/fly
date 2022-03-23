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

ASTType::ASTType(SourceLocation Loc, TypeKind Kind) : Loc(Loc), Kind(Kind) {}

const SourceLocation &ASTType::getLocation() const  {
    return Loc;
}

const TypeKind &ASTType::getKind() const  {
    return Kind;
}

bool ASTType::equals(ASTType *Ty) const {
    return this->getKind() == Ty->getKind();
}

ASTVoidType::ASTVoidType(SourceLocation Loc) : ASTType(Loc, TypeKind::TYPE_VOID) {

}

ASTBoolType::ASTBoolType(SourceLocation Loc) : ASTType(Loc, TypeKind::TYPE_BOOL) {

}

ASTByteType::ASTByteType(SourceLocation Loc) : ASTType(Loc, TypeKind::TYPE_BYTE) {

}

ASTUShortType::ASTUShortType(SourceLocation Loc) : ASTType(Loc, TypeKind::TYPE_USHORT) {

}

ASTShortType::ASTShortType(SourceLocation Loc) : ASTType(Loc, TypeKind::TYPE_SHORT) {

}

ASTUIntType::ASTUIntType(SourceLocation Loc)  : ASTType(Loc, TypeKind::TYPE_UINT) {

}

ASTIntType::ASTIntType(SourceLocation Loc)  : ASTType(Loc, TypeKind::TYPE_INT) {

}

ASTULongType::ASTULongType(SourceLocation Loc)  : ASTType(Loc, TypeKind::TYPE_ULONG) {

}

ASTLongType::ASTLongType(SourceLocation Loc)  : ASTType(Loc, TypeKind::TYPE_LONG) {

}

ASTFloatType::ASTFloatType(SourceLocation Loc) : ASTType(Loc, TypeKind::TYPE_FLOAT) {

}

ASTDoubleType::ASTDoubleType(SourceLocation Loc) : ASTType(Loc, TypeKind::TYPE_DOUBLE) {

}

ASTClassType::ASTClassType(SourceLocation Loc, std::string Name, std::string NameSpace) :
    ASTType(Loc, TypeKind::TYPE_CLASS), Name(Name) {

}

const std::string &ASTClassType::getName() const {
    return Name;
}

bool ASTClassType::operator==(const ASTClassType &Ty) const {
    return getKind() == Ty.getKind() && Name == Ty.Name;
}
