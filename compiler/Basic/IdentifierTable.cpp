//===- IdentifierTable.cpp - Hash table for identifier lookup -------------===//
//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/AddressSpaces.h - Language-specific address spaces
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// This file implements the IdentifierInfo, IdentifierVisitor, and
// IdentifierTable interfaces.
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Basic/IdentifierTable.h"
#include "Basic/CharInfo.h"
#include "Basic/Specifiers.h"
#include "Basic/TokenKinds.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

using namespace fly;

//===--------------------------------------------------------------------------------------------------------------===//
// IdentifierTable Implementation
//===--------------------------------------------------------------------------------------------------------------===//

IdentifierIterator::~IdentifierIterator() = default;

IdentifierInfoLookup::~IdentifierInfoLookup() = default;

namespace {

/// A simple identifier lookup iterator that represents an
/// empty sequence of identifiers.
    class EmptyLookupIterator : public IdentifierIterator
    {
    public:
        StringRef Next() override { return StringRef(); }
    };

} // namespace

IdentifierIterator *IdentifierInfoLookup::getIdentifiers() {
    return new EmptyLookupIterator();
}

IdentifierTable::IdentifierTable(IdentifierInfoLookup *ExternalLookup)
        : HashTable(8192), // Start with space for 8K identifiers.
          ExternalLookup(ExternalLookup) {
    AddKeywords();
}

//===--------------------------------------------------------------------------------------------------------------===//
// Language Keyword Implementation
//===--------------------------------------------------------------------------------------------------------------===//

/// AddKeyword - This method is used to associate a token ID with specific
/// identifiers because they are language keywords.  This causes the lexer to
/// automatically map matching identifiers to specialized token codes.
static void AddKeyword(StringRef Keyword,
                       tok::TokenKind TokenCode, IdentifierTable &Table) {

    IdentifierInfo &Info = Table.get(Keyword, TokenCode);
}

/// AddKeywords - Add all keywords to the symbol table.
///
void IdentifierTable::AddKeywords() {
    // Add keywords and tokens for the current language.
#define KEYWORD(NAME) \
  AddKeyword(StringRef(#NAME), tok::kw_ ## NAME, *this);
#include "Basic/TokenKinds.def"

}

//===--------------------------------------------------------------------------------------------------------------===//
// Stats Implementation
//===--------------------------------------------------------------------------------------------------------------===//

/// PrintStats - Print statistics about how well the identifier table is doing
/// at hashing identifiers.
void IdentifierTable::PrintStats() const {
    unsigned NumBuckets = HashTable.getNumBuckets();
    unsigned NumIdentifiers = HashTable.getNumItems();
    unsigned NumEmptyBuckets = NumBuckets-NumIdentifiers;
    unsigned AverageIdentifierSize = 0;
    unsigned MaxIdentifierLength = 0;

    // TODO: Figure out maximum times an identifier had to probe for -stats.
    for (llvm::StringMap<IdentifierInfo*, llvm::BumpPtrAllocator>::const_iterator
                 I = HashTable.begin(), E = HashTable.end(); I != E; ++I) {
        unsigned IdLen = I->getKeyLength();
        AverageIdentifierSize += IdLen;
        if (MaxIdentifierLength < IdLen)
            MaxIdentifierLength = IdLen;
    }

    fprintf(stderr, "\n*** Identifier Table Stats:\n");
    fprintf(stderr, "# Identifiers:   %d\n", NumIdentifiers);
    fprintf(stderr, "# Empty Buckets: %d\n", NumEmptyBuckets);
    fprintf(stderr, "Hash density (#identifiers per bucket): %f\n",
            NumIdentifiers/(double)NumBuckets);
    fprintf(stderr, "Ave identifier length: %f\n",
            (AverageIdentifierSize/(double)NumIdentifiers));
    fprintf(stderr, "Max identifier length: %d\n", MaxIdentifierLength);

    // Compute statistics about the memory allocated for identifiers.
    HashTable.getAllocator().PrintStats();
}
