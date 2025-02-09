//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBase.cpp - AST Base implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBase.h"

using namespace fly;

ASTBase::ASTBase(const SourceLocation &Loc, ASTKind Kind) : Location(Loc), Kind(Kind) {

}

const SourceLocation &ASTBase::getLocation() const {
    return Location;
}

ASTKind ASTBase::getKind() const {
	return Kind;
}

std::string ASTBase::str() const {
    return Logger("ASTBase").
            Attr("Location", static_cast<uint64_t>(Location.getRawEncoding())).
			Attr("Kind", static_cast<uint64_t>(Kind)).
            End();
}
