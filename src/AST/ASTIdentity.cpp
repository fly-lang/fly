//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentity.cpp - AST Identity implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTIdentity.h"
#include "AST/ASTModule.h"

using namespace fly;

ASTIdentity::ASTIdentity(ASTTopDefKind TopDefKind, ASTScopes *Scopes,const SourceLocation &Loc,
                         llvm::StringRef Name) :
        ASTBase(Loc), TopDefKind(TopDefKind), Scopes(Scopes), Name(Name) {

}

ASTTopDefKind ASTIdentity::getTopDefKind() const {
    return TopDefKind;
}

llvm::StringRef ASTIdentity::getName() const {
    return Name;
}

ASTScopes *ASTIdentity::getScopes() const {
    return Scopes;
}

ASTModule *ASTIdentity::getModule() const {
    return Module;
}

ASTNameSpace *ASTIdentity::getNameSpace() const {
    return Module->getNameSpace();
}

std::string ASTIdentity::str() const {
    return Logger("ASTIdentity").
            Super(ASTBase::str()).
            Attr("Name", Name).
            Attr("TopDefKind", (uint64_t) TopDefKind).
            Attr("Scopes", Scopes).
            End();
}
