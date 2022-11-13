//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentifier.cpp - Identifier implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTIdentifier.h"

using namespace fly;

ASTIdentifier::ASTIdentifier(ASTIdKind Kind, const SourceLocation &Loc, llvm::StringRef NameSpace, llvm::StringRef Name) :
    Kind(Kind), Loc(Loc), NameSpace(NameSpace), Name(Name) {
}

ASTIdentifier::ASTIdentifier(ASTIdKind Kind, const SourceLocation &Loc, llvm::StringRef NameSpace, llvm::StringRef ClassName, llvm::StringRef Name) :
        Kind(Kind), Loc(Loc), NameSpace(NameSpace), ClassName(ClassName), Name(Name) {
}

ASTIdentifier::ASTIdentifier(const SourceLocation &Loc, llvm::StringRef NameSpace, llvm::StringRef Name) :
        Kind(ASTIdKind::UNDEFINED), Loc(Loc), NameSpace(NameSpace), Name(Name) {
}

ASTIdentifier::ASTIdentifier(const SourceLocation &Loc, llvm::StringRef NameSpace, llvm::StringRef ClassName, llvm::StringRef Name) :
        Kind(ASTIdKind::UNDEFINED), Loc(Loc), NameSpace(NameSpace), ClassName(ClassName), Name(Name) {
}

const ASTIdentifier::ASTIdKind ASTIdentifier::getKind() const {
    return Kind;
}

const SourceLocation &ASTIdentifier::getLocation() const {
    return Loc;
}

llvm::StringRef ASTIdentifier::getNameSpace() const {
    return NameSpace;
}

llvm::StringRef ASTIdentifier::getClassName() const {
    return ClassName;
}

llvm::StringRef ASTIdentifier::getName() const {
    return Name;
}

std::string ASTIdentifier::str() const {
    std::string StrName;
    return Logger("ASTIdentifier").
            Attr("Location", Loc).
            Attr("NameSpace", NameSpace).
            Attr("ClassName", ClassName).
            Attr("Name", Name).
            End();
}
