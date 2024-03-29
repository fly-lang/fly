//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.cpp - Var declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVarRef.h"
#include "AST/ASTVar.h"

using namespace fly;

ASTVarRef::ASTVarRef(const SourceLocation &Loc, llvm::StringRef Name) :
        ASTIdentifier(Loc, Name) {

}

ASTVarRef::ASTVarRef(const SourceLocation &Loc, llvm::StringRef ClassName, llvm::StringRef Name) :
        ASTIdentifier(Loc, ClassName, Name) {

}

ASTVar *ASTVarRef::getDef() const {
    return Def;
}

ASTVar *ASTVarRef::getInstance() const {
    return Instance;
}

std::string ASTVarRef::str() const {
    return Logger("ASTVarRef").
            Attr("Location", Loc).
            Attr("NameSpace", NameSpace).
            Attr("Class", ClassName).
            Attr("Name", Name).
            Attr("Def", Def).
            End();
}
