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
#include "CodeGen/CodeGenEnum.h"

using namespace fly;

ASTEnum::ASTEnum(const SourceLocation &Loc, llvm::StringRef Name, ASTScopes *Scopes,
                   llvm::SmallVector<ASTEnumType *, 4> &ExtClasses) :
        ASTIdentity(ASTTopDefKind::DEF_ENUM, Scopes, Loc, Name),
        SuperClasses(ExtClasses) {

}

llvm::StringMap<ASTEnumEntry *> ASTEnum::getEntries() const {
    return Entries;
}

CodeGenEnum *ASTEnum::getCodeGen() const {
    return CodeGen;
}

void ASTEnum::setCodeGen(CodeGenEnum *CGE) {
    CodeGen = CGE;
}

std::string ASTEnum::print() const {
    std::string ClassName = Name.data();
    return getNameSpace()->print() + "." + ClassName;
}

std::string ASTEnum::str() const {

    // Fields to string
    std::string EnumList;
    for (auto &Enum : Entries) {
        std::string EQ = "=";
        EnumList += Enum.getKey().data() + EQ + Enum.getValue()->str() + ",";
    }
    EnumList.substr(0, EnumList.length()-1);

    // Class to string
    return Logger("ASTClass").
           Super(ASTIdentity::str()).
           Attr("Name", Name).
           Attr("Scopes", Scopes).
           Attr("Vars", EnumList).
           End();
}
