//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaIdentitySymbols.cpp - The Sema identity Symbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaIdentitySymbols.h"

#include "llvm/ADT/StringMap.h"

using namespace fly;

SemaIdentitySymbols::SemaIdentitySymbols(ASTIdentity *Identity) : Identity(Identity) {

}

ASTIdentity *SemaIdentitySymbols::getIdentity() {
    return Identity;
}

llvm::StringMap<ASTClassAttribute *> SemaIdentitySymbols::getAttributes() {
    return Attributes;
}

const llvm::StringMap<ASTEnumEntry *> &SemaIdentitySymbols::getEntries() const {
    return Entries;
}

llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTClassMethod *, 4>>> SemaIdentitySymbols::getMethods() {
    return Methods;
}


