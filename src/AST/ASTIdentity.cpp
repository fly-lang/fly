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
#include "AST/ASTScopes.h"

using namespace fly;

ASTIdentity::ASTIdentity(ASTModule *Module, ASTTopDefKind TopDefKind, llvm::SmallVector<ASTScope *, 8> &Scopes,
                         const SourceLocation &Loc, llvm::StringRef Name) :
        ASTBase(Loc), Module(Module), TopDefKind(TopDefKind), Scopes(Scopes), Name(Name) {

}

ASTTopDefKind ASTIdentity::getTopDefKind() const {
    return TopDefKind;
}

ASTIdentityType *ASTIdentity::getType() const {
    return Type;
}

llvm::StringRef ASTIdentity::getName() const {
    return Name;
}

ASTVisibilityKind ASTIdentity::getVisibility() const {
    return Visibility;
}

llvm::SmallVector<ASTScope *, 8> ASTIdentity::getScopes() const {
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
            AttrList("Scopes", Scopes).
            End();
}
