//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaSymbols.cpp - The Sema Symbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaSpaceSymbols.h"
#include "Sema/Sema.h"

#include "llvm/ADT/StringMap.h"

using namespace fly;

SemaSpaceSymbols::SemaSpaceSymbols(Sema &S) : S(S) {

}

const llvm::StringMap<ASTGlobalVar *> &SemaSpaceSymbols::getGlobalVars() const {
    return GlobalVars;
}

const llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>>> &SemaSpaceSymbols::getFunctions() const {
    return Functions;
}

const llvm::StringMap<SemaIdentitySymbols *> &SemaSpaceSymbols::getIdentities() const {
    return Identities;
}

