//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTComment.cpp - AST Comment
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTComment.h"

using namespace fly;

ASTComment::ASTComment(const SourceLocation &Loc, llvm::StringRef Content) : ASTBase(Loc),
    Content(Content) {

}

llvm::StringRef ASTComment::getContent() const {
    return Content;
}

std::string ASTComment::str() const {
    return Logger("ASTComment").
            Super(ASTBase::str()).
            Attr("Content", Content).
            End();
}
