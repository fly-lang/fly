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

ASTType::ASTType(SourceLocation Loc) : Loc(Loc) {}

const SourceLocation &ASTType::getLocation() const  {
    return Loc;
}

bool ASTType::equals(ASTType *Ty) const {
    return this->getKind() == Ty->getKind();
}

ASTIntType::ASTIntType(SourceLocation Loc)  : ASTType(Loc) {

}

const TypeKind &ASTIntType::getKind() const {
    return Kind;
}

ASTFloatType::ASTFloatType(SourceLocation Loc) : ASTType(Loc) {

}

const TypeKind &ASTFloatType::getKind() const {
    return Kind;
}

ASTBoolType::ASTBoolType(SourceLocation Loc) : ASTType(Loc) {

}

const TypeKind &ASTBoolType::getKind() const  {
    return Kind;
}

ASTVoidType::ASTVoidType(SourceLocation Loc) : ASTType(Loc) {

}

const TypeKind &ASTVoidType::getKind() const {
    return Kind;
}

ASTClassType::ASTClassType(SourceLocation Loc, StringRef &Name)  : ASTType(Loc), Name(Name) {

}

const TypeKind &ASTClassType::getKind() const {
    return Kind;
}

const llvm::StringRef &ASTClassType::getName() const {
    return Name;
}

bool ASTClassType::operator==(const ASTClassType &Ty) const {
    return getKind() == Ty.getKind() && Name.equals(Ty.Name);
}
