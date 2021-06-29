//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNodeBase.cpp - Base AST Node implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTNodeBase.h"

using namespace fly;

ASTNodeBase::ASTNodeBase(const llvm::StringRef &FileName, const FileID &FID, ASTContext *Context) :
        FileName(FileName), FID(FID), Context(Context) {
}

const ASTContext &ASTNodeBase::getContext() const {
    return *Context;
}

const llvm::StringRef &ASTNodeBase::getFileName() {
    return FileName;
}

const FileID &ASTNodeBase::getFileID() const {
    return FID;
}
