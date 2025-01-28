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

ASTEnum::ASTEnum(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name,
                 llvm::SmallVector<ASTScope *, 8> &Scopes, llvm::SmallVector<ASTEnumType *, 4> &SuperClasses) :
        ASTIdentity(Module, ASTIdentityKind ::ID_ENUM, Scopes, Loc, Name),
        SuperClasses(SuperClasses) {

}

ASTModule * ASTEnum::getModule() const {
	return Module;
}

llvm::SmallVector<ASTEnumEntry *, 8> ASTEnum::getEntries() const {
    return Entries;
}

std::string ASTEnum::str() const {

    // Class to string
    return Logger("ASTClass").
           Super(ASTIdentity::str()).
           Attr("Name", Name).
           AttrList("Scopes", Scopes).
           AttrList("Entries", Entries).
           End();
}
