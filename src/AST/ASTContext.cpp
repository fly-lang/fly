//===----------------------------------------------------------------------===//
// AST/ASTContext.cpp - AST Context implementation
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
//
// This file implements the ASTContext interface.
//
//===----------------------------------------------------------------------===//
#include "AST/ASTContext.h"

using namespace fly;
using namespace std;

ASTContext::ASTContext(const string &fileName, const PackageDecl &packageDecl) :
        fileName(move(fileName)), package(move(packageDecl)) {

}

const string &ASTContext::getFileName() {
    return fileName;
}

const PackageDecl &ASTContext::getPackage() {
    return package;
}

void ASTContext::Release() {

}
