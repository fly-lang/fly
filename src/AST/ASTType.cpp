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

IntPrimType::IntPrimType(SourceLocation Loc)  : ASTType(Loc) {

}

const TypeKind &IntPrimType::getKind() const {
    return Kind;
}

FloatPrimType::FloatPrimType(SourceLocation Loc) : ASTType(Loc) {

}

const TypeKind &FloatPrimType::getKind() const {
    return Kind;
}

BoolPrimType::BoolPrimType(SourceLocation Loc) : ASTType(Loc) {

}

const TypeKind &BoolPrimType::getKind() const  {
    return Kind;
}

VoidRetType::VoidRetType(SourceLocation Loc) : ASTType(Loc) {

}

const TypeKind &VoidRetType::getKind() const {
    return Kind;
}

ClassTypeRef::ClassTypeRef(SourceLocation Loc, StringRef &Name)  : ASTType(Loc), Name(Name) {

}

const TypeKind &ClassTypeRef::getKind() const {
    return Kind;
}

const llvm::StringRef &ClassTypeRef::getName() const {
    return Name;
}

bool ClassTypeRef::operator==(const ClassTypeRef &Ty) const {
    return getKind() == Ty.getKind() && Name.equals(Ty.Name);
}
