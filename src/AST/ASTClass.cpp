//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClass.cpp - Class implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClass.h"

using namespace fly;

ASTClass::ASTClass(const SourceLocation &Loc, ASTNode *Node, const std::string Name,
                   VisibilityKind Visibility, bool Constant) :
        ASTTopDef(Loc, Node, TopDeclKind::DECL_CLASS, Visibility), Constant(Constant) {

}

const std::string ASTClass::getName() const {
    return Name;
}