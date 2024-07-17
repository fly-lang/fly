//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTEnum.cpp - AST Enum implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTEnum.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTScopes.h"
#include "AST/ASTNameSpace.h"

using namespace fly;

ASTEnum::ASTEnum(const SourceLocation &Loc, llvm::StringRef Name, ASTScopes *Scopes,
                   llvm::SmallVector<ASTEnumType *, 4> &ExtClasses) :
        ASTIdentity(ASTTopDefKind::DEF_ENUM, Scopes, Loc, Name),
        SuperClasses(ExtClasses) {

}

llvm::SmallVector<ASTEnumEntry *, 8> ASTEnum::getEntries() const {
    return Entries;
}

std::string ASTEnum::str() const {

    // Class to string
    return Logger("ASTClass").
           Super(ASTIdentity::str()).
           Attr("Name", Name).
           Attr("Scopes", Scopes).
           AttrList("Entries", Entries).
           End();
}
