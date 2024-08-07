//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaSymbols.cpp - The Sema Symbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaSymbols.h"
#include "Sema/Sema.h"
#include "Basic/Debug.h"

#include "llvm/ADT/StringMap.h"

using namespace fly;

SemaSymbols::SemaSymbols(Sema &S) : S(S) {

}

const llvm::StringMap<ASTGlobalVar *> &SemaSymbols::getGlobalVars() const {
    return GlobalVars;
}

const llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>>> &SemaSymbols::getFunctions() const {
    return Functions;
}

const llvm::StringMap<ASTIdentity *> &SemaSymbols::getIdentities() const {
    return Identities;
}

SemaSymbols *SemaSymbols::FindImport(llvm::StringRef Name, ) {
    // Search into Module imports
    ASTImport *Import = nullptr;
    for (auto &I : Module->Imports)
        if (I->getName() == Name)
            Import = I;
    if (Import == nullptr)
        for (auto &AliasImport :Module->AliasImports)
            if (AliasImport->getName() == Name)
                Import = AliasImport;
    return Import;
}

void doSome() {
//    for (auto &StrMapEntry : Functions) {
//        for (auto &IntMap: StrMapEntry.getValue()) {
//            for (auto &Function: IntMap.second) {
//
//            }
//        }
//    }

            // TODO
            // Lookup into namespace for public var
//                if(Function->getVisibility() == ASTVisibilityKind::V_PUBLIC ||
//                   Function->getVisibility() == ASTVisibilityKind::V_DEFAULT) {
//
//                    // Add into NameSpace for global resolution
//                    // Add into Module for local resolution
//                    return InsertFunction(Module->NameSpace->Functions, Function) &&
//                           InsertFunction(Module->Functions, Function);
//                }
//
//                // Lookup into Module for private var
//                if (Function->getVisibility() == ASTVisibilityKind::V_PRIVATE) {
//
//                    // Add into Module for local resolution
//                    return InsertFunction(Module->Functions, Function);
//                }

}

