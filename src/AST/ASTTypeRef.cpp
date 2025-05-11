//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTTypeRef.cpp - AST Type Ref implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTTypeRef.h"
#include "Basic/Logger.h"

using namespace fly;

ASTTypeRef::ASTTypeRef(const SourceLocation &Loc, llvm::StringRef Name, ASTNameSpaceRef *NameSpaceRef, bool Array) :
        ASTRef(Loc, Name, ASTRefKind::REF_TYPE), NameSpaceRef(NameSpaceRef), Array(Array) {

}

SemaType * ASTTypeRef::getSema() const {
	return Sema;
}

bool ASTTypeRef::isArray() const {
	return Array;
}

ASTNameSpaceRef *ASTTypeRef::getNameSpaceRef() const {
	return NameSpaceRef;
}

std::string ASTTypeRef::str() const {
    return Logger("ASTTypeRef").
	Attr("Location", getLocation()).
	Attr("Kind", static_cast<size_t>(getKind())).
    End();
}

ASTArrayTypeRef::ASTArrayTypeRef(const SourceLocation &Loc, ASTTypeRef *TypeRef, llvm::StringRef Name) :
	ASTTypeRef(Loc, Name, TypeRef->getNameSpaceRef(), true) {
}

ASTTypeRef * ASTArrayTypeRef::getTypeRef() const {
	return TypeRef;
}

std::string ASTArrayTypeRef::str() const {
	return ASTTypeRef::str();
}
