//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVarRef.cpp - AST Var Ref implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVarRef.h"
#include "AST/ASTVar.h"
#include "Basic/Logger.h"

using namespace fly;

ASTVarRef::ASTVarRef(const SourceLocation &Loc, llvm::StringRef Name) : ASTRef(Loc, Name, ASTRefKind::REF_VAR) {

}

SymVar *ASTVarRef::getVar() const {
    return *Var;
}

std::string ASTVarRef::str() const {
    return Logger("ASTVarRef").
	Attr("Location", getLocation()).
	Attr("Kind", static_cast<size_t>(getKind())).
            Attr("Parent", Parent).
            Attr("Def", Var).
            End();
}
