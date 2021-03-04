//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTContext.cpp - AST Context implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// This file implements the ASTContext interface.
//
//===--------------------------------------------------------------------------------------------------------------===//
#include "AST/ASTContext.h"

using namespace fly;
using namespace std;

ASTContext::ASTContext(const StringRef &fileName, const PackageDecl &packageDecl) :
        fileName(move(fileName)), package(move(packageDecl)) {

}

const StringRef &ASTContext::getFileName() const {
    return fileName;
}

const PackageDecl &ASTContext::getPackage() {
    return package;
}

void ASTContext::Release() {

}
