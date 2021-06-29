//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/Type.h - Type implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/TypeBase.h"

using namespace fly;

TypeBase::TypeBase(SourceLocation Loc) : Loc(Loc) {}

const SourceLocation &TypeBase::getLocation() const  {
    return Loc;
}

bool TypeBase::operator==(const TypeBase &Ty) const {
    return getKind() == Ty.getKind();
}

IntPrimType::IntPrimType(SourceLocation Loc)  : TypeBase(Loc) {

}

const TypeKind &IntPrimType::getKind() const {
    return Kind;
}

FloatPrimType::FloatPrimType(SourceLocation Loc) : TypeBase(Loc) {

}

const TypeKind &FloatPrimType::getKind() const {
    return Kind;
}

BoolPrimType::BoolPrimType(SourceLocation Loc) : TypeBase(Loc) {

}

const TypeKind &BoolPrimType::getKind() const  {
    return Kind;
}

VoidRetType::VoidRetType(SourceLocation Loc) : TypeBase(Loc) {

}

const TypeKind &VoidRetType::getKind() const {
    return Kind;
}

ClassTypeRef::ClassTypeRef(SourceLocation Loc, StringRef &Name)  : TypeBase(Loc), Name(Name) {

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
