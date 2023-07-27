//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTEnum.cpp - Enum implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTEnum.h"
#include "AST/ASTEnumVar.h"
#include "AST/ASTScopes.h"
#include "AST/ASTNameSpace.h"
#include "CodeGen/CodeGenEnum.h"

using namespace fly;

ASTEnum::ASTEnum(ASTNode *Node, ASTScopes *Scopes, const SourceLocation &Loc, llvm::StringRef Name,
                   llvm::SmallVector<ASTEnumType *, 4> &ExtClasses) :
        ASTIdentity(Node, ASTTopDefKind::DEF_ENUM, Scopes, Loc, Name),
        SuperClasses(ExtClasses) {

}

ASTEnumType *ASTEnum::getType() const {
    return Type;
}

llvm::StringMap<ASTEnumVar *> ASTEnum::getVars() const {
    return Vars;
}

CodeGenEnum *ASTEnum::getCodeGen() const {
    return CodeGen;
}

void ASTEnum::setCodeGen(CodeGenEnum *CGE) {
    CodeGen = CGE;
}

std::string ASTEnum::print() const {
    std::string ClassName = Name.data();
    return NameSpace->print() + "." + ClassName;
}

std::string ASTEnum::str() const {

    // Fields to string
    std::string EnumList;
    for (auto &Enum : Vars) {
        std::string EQ = "=";
        EnumList += Enum.getKey().data() + EQ + Enum.getValue()->str() + ",";
    }
    EnumList.substr(0, EnumList.length()-1);

    // Class to string
    return Logger("ASTClass").
           Super(ASTTopDef::str()).
           Attr("Name", Name).
           Attr("Scopes", Scopes).
           Attr("Vars", EnumList).
           End();
}
