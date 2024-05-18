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

using namespace fly;

ASTVarRef::ASTVarRef(const SourceLocation &Loc, llvm::StringRef Name) : ASTIdentifier(Loc, Name, ASTIdentifierKind::REF_VAR) {

}

ASTVar *ASTVarRef::getDef() const {
    return Def;
}

std::string ASTVarRef::print() const {
    return Def ? Def->print() : ASTIdentifier::print();
}

std::string ASTVarRef::str() const {
    return Logger("ASTVarRef").
            Attr("Parent", Parent).
            Attr("Def", Def).
            End();
}
