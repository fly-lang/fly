//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBase.cpp - Base AST
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTNodeBase.h"

using namespace fly;

ASTNodeBase::ASTNodeBase(const StringRef &FileName, const FileID &FID, ASTContext *Context) :
        FileName(FileName), FID(FID), Context(Context) {
}

ASTContext &ASTNodeBase::getContext() const {
    return *Context;
}

const llvm::StringRef &ASTNodeBase::getFileName() {
    return FileName;
}

const FileID &ASTNodeBase::getFileID() const {
    return FID;
}

llvm::DenseMap<FileID, ASTNode*> &ASTNodeBase::getDependencies() {
    return Dependencies;
}

const llvm::DenseMap<FileID, ASTNode*> &ASTNodeBase::getCallers() const {
    return Callers;
}
