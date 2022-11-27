//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClassMethod.cpp - Class Method implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClassFunction.h"
#include "AST/ASTClass.h"

using namespace fly;

ASTClassFunction::ASTClassFunction(const SourceLocation &Loc, ASTClass *Class, ASTClassScopes *Scopes, ASTType *Type,
                                   llvm::StringRef Name) :
        ASTFunctionBase(Loc, ASTFunctionKind::CLASS_FUNCTION, Type, Name), Class(Class), Scopes(Scopes)  {

}

ASTClass *ASTClassFunction::getClass() const {
    return Class;
}

llvm::StringRef ASTClassFunction::getComment() const {
    return Comment;
}

ASTClassScopes *ASTClassFunction::getScopes() const {
    return Scopes;
}

std::string ASTClassFunction::str() const {
    return Logger("ASTClassFunction").
           Super(ASTFunctionBase::str()).
            Attr("Class", Class).
            Attr("Comment", Comment).
            Attr("Scopes", Scopes).
            End();
}
