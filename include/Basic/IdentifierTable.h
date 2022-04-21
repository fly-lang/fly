//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/IdentifierTable.h - Hash table for identifier lookup
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
/// \file
/// Defines the fly::IdentifierInfo, fly::IdentifierTable, and
/// fly::Selector interfaces.
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_BASIC_IDENTIFIERTABLE_H
#define FLY_BASIC_IDENTIFIERTABLE_H

#include "Basic/LLVM.h"
#include "Basic/TokenKinds.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/PointerLikeTypeTraits.h"
#include "llvm/Support/type_traits.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

namespace fly {

    class IdentifierInfo;
    class MultiKeywordSelector;
    class SourceLocation;

/// A simple pair of identifier info and location.
    using IdentifierLocPair = std::pair<IdentifierInfo *, SourceLocation>;

/// IdentifierInfo and other related classes are aligned to
/// 8 bytes so that DeclarationName can use the lower 3 bits
/// of a pointer to one of these classes.
    enum { IdentifierInfoAlignment = 8 };

/// One of these records is kept for each identifier that
/// is lexed.  This contains information about whether the token was \#define'd,
/// is a language keyword, or if it is a front-end token of some sort (e.g. a
/// variable or function name).  The preprocessor keeps this information in a
/// set, and all tok::identifier tokens have a pointer to one of these.
/// It is aligned to 8 bytes because DeclarationName needs the lower 3 bits.
    class alignas(IdentifierInfoAlignment) IdentifierInfo {
        friend class IdentifierTable;

        // Front-end token ID or tok::identifier.
        unsigned TokenID : 9;

        // True if the identifier is poisoned.
        unsigned IsPoisoned : 1;

        // Internal bit set by the member function RecomputeNeedsHandleIdentifier.
        // See comment about RecomputeNeedsHandleIdentifier for more info.
        unsigned NeedsHandleIdentifier : 1;

        // True if the identifier was loaded (at least partially) from an AST file.
        unsigned IsFromAST : 1;

        // True if the identifier has changed from the definition
        // loaded from an AST file.
        unsigned ChangedAfterLoad : 1;

        // True if the identifier's frontend information has changed from the
        // definition loaded from an AST file.
        unsigned FEChangedAfterLoad : 1;

        // 29 bits left in a 64-bit word.

        // Managed by the language front-end.
        void *FETokenInfo = nullptr;

        llvm::StringMapEntry<IdentifierInfo *> *Entry = nullptr;

        IdentifierInfo()
                : TokenID(tok::identifier), IsPoisoned(false),
                  NeedsHandleIdentifier(false), IsFromAST(false), ChangedAfterLoad(false),
                  FEChangedAfterLoad(false) {}

    public:
        IdentifierInfo(const IdentifierInfo &) = delete;
        IdentifierInfo &operator=(const IdentifierInfo &) = delete;
        IdentifierInfo(IdentifierInfo &&) = delete;
        IdentifierInfo &operator=(IdentifierInfo &&) = delete;

        /// Return true if this is the identifier for the specified string.
        ///
        /// This is intended to be used for string literals only: II->isStr("foo").
        template <std::size_t StrLen>
        bool isStr(const char (&Str)[StrLen]) const {
            return getLength() == StrLen-1 &&
                   memcmp(getNameStart(), Str, StrLen-1) == 0;
        }

        /// Return true if this is the identifier for the specified StringRef.
        bool isStr(llvm::StringRef Str) const {
            llvm::StringRef ThisStr(getNameStart(), getLength());
            return ThisStr == Str;
        }

        /// Return the beginning of the actual null-terminated string for this
        /// identifier.
        const char *getNameStart() const { return Entry->getKeyData(); }

        /// Efficiently return the length of this identifier info.
        unsigned getLength() const { return Entry->getKeyLength(); }

        /// Return the actual identifier string.
        StringRef getName() const {
            return StringRef(getNameStart(), getLength());
        }

        /// If this is a source-language token (e.g. 'for'), this API
        /// can be used to cause the lexer to map identifiers to source-language
        /// tokens.
        tok::TokenKind getTokenID() const { return (tok::TokenKind)TokenID; }

        /// setIsPoisoned - Mark this identifier as poisoned.  After poisoning, the
        /// Preprocessor will emit an error every time this token is used.
        void setIsPoisoned(bool Value = true) {
            IsPoisoned = Value;
            if (Value)
                NeedsHandleIdentifier = true;
            else
                RecomputeNeedsHandleIdentifier();
        }

        /// Return true if this token has been poisoned.
        bool isPoisoned() const { return IsPoisoned; }

        /// Return true if this token is a keyword in the specified language.
        bool isKeyword() const;

        /// Get and set FETokenInfo. The language front-end is allowed to associate
        /// arbitrary metadata with this token.
        void *getFETokenInfo() const { return FETokenInfo; }
        void setFETokenInfo(void *T) { FETokenInfo = T; }

        /// Return true if the Preprocessor::HandleIdentifier must be called
        /// on a token of this identifier.
        ///
        /// If this returns false, we know that HandleIdentifier will not affect
        /// the token.
        bool isHandleIdentifierCase() const { return NeedsHandleIdentifier; }

        /// Return true if the identifier in its current state was loaded
        /// from an AST file.
        bool isFromAST() const { return IsFromAST; }

        void setIsFromAST() { IsFromAST = true; }

        /// Determine whether this identifier has changed since it was loaded
        /// from an AST file.
        bool hasChangedSinceDeserialization() const {
            return ChangedAfterLoad;
        }

        /// Note that this identifier has changed since it was loaded from
        /// an AST file.
        void setChangedSinceDeserialization() {
            ChangedAfterLoad = true;
        }

        /// Determine whether the frontend token information for this
        /// identifier has changed since it was loaded from an AST file.
        bool hasFETokenInfoChangedSinceDeserialization() const {
            return FEChangedAfterLoad;
        }

        /// Note that the frontend token information for this identifier has
        /// changed since it was loaded from an AST file.
        void setFETokenInfoChangedSinceDeserialization() {
            FEChangedAfterLoad = true;
        }

        /// Return true if this identifier is an editor placeholder.
        ///
        /// Editor placeholders are produced by the code-completion engine and are
        /// represented as characters between '<#' and '#>' in the source code. An
        /// example of auto-completed call with a placeholder parameter is shown
        /// below:
        /// \code
        ///   function(<#int x#>);
        /// \endcode
        bool isEditorPlaceholder() const {
            return getName().startswith("<#") && getName().endswith("#>");
        }

        /// Determine whether \p this is a name reserved for the implementation (C99
        /// 7.1.3, C++ [lib.global.names]).
        bool isReservedName(bool doubleUnderscoreOnly = false) const {
            if (getLength() < 2)
                return false;
            const char *Name = getNameStart();
            return Name[0] == '_' &&
                   (Name[1] == '_' ||
                    (Name[1] >= 'A' && Name[1] <= 'Z' && !doubleUnderscoreOnly));
        }

        /// Provide less than operator for lexicographical sorting.
        bool operator<(const IdentifierInfo &RHS) const {
            return getName() < RHS.getName();
        }

    private:
        /// The Preprocessor::HandleIdentifier does several special (but rare)
        /// things to identifiers of various sorts.  For example, it changes the
        /// \c for keyword token from tok::identifier to tok::for.
        ///
        /// This method is very tied to the definition of HandleIdentifier.  Any
        /// change to it should be reflected here.
        void RecomputeNeedsHandleIdentifier() {
            NeedsHandleIdentifier = isPoisoned();
        }
    };

/// An RAII object for [un]poisoning an identifier within a scope.
///
/// \p II is allowed to be null, in which case objects of this type have
/// no effect.
    class PoisonIdentifierRAIIObject {
        IdentifierInfo *const II;
        const bool OldValue;

    public:
        PoisonIdentifierRAIIObject(IdentifierInfo *II, bool NewValue)
                : II(II), OldValue(II ? II->isPoisoned() : false) {
            if(II)
                II->setIsPoisoned(NewValue);
        }

        ~PoisonIdentifierRAIIObject() {
            if(II)
                II->setIsPoisoned(OldValue);
        }
    };

/// An iterator that walks over all of the known identifiers
/// in the lookup table.
///
/// Since this iterator uses an abstract interface via virtual
/// functions, it uses an object-oriented interface rather than the
/// more standard C++ STL iterator interface. In this OO-style
/// iteration, the single function \c Next() provides dereference,
/// advance, and end-of-sequence checking in a single
/// operation. Subclasses of this iterator type will provide the
/// actual functionality.
    class IdentifierIterator {
    protected:
        IdentifierIterator() = default;

    public:
        IdentifierIterator(const IdentifierIterator &) = delete;
        IdentifierIterator &operator=(const IdentifierIterator &) = delete;

        virtual ~IdentifierIterator();

        /// Retrieve the next string in the identifier table and
        /// advances the iterator for the following string.
        ///
        /// \returns The next string in the identifier table. If there is
        /// no such string, returns an empty \c StringRef.
        virtual StringRef Next() = 0;
    };

/// Provides lookups to, and iteration over, IdentiferInfo objects.
    class IdentifierInfoLookup {
    public:
        virtual ~IdentifierInfoLookup();

        /// Return the IdentifierInfo for the specified named identifier.
        ///
        /// Unlike the version in IdentifierTable, this returns a pointer instead
        /// of a reference.  If the pointer is null then the IdentifierInfo cannot
        /// be found.
        virtual IdentifierInfo* get(StringRef Name) = 0;

        /// Retrieve an iterator into the set of all identifiers
        /// known to this identifier lookup source.
        ///
        /// This routine provides access to all of the identifiers known to
        /// the identifier lookup, allowing access to the contents of the
        /// identifiers without introducing the overhead of constructing
        /// IdentifierInfo objects for each.
        ///
        /// \returns A new iterator into the set of known identifiers. The
        /// caller is responsible for deleting this iterator.
        virtual IdentifierIterator *getIdentifiers();
    };

/// Implements an efficient mapping from strings to IdentifierInfo nodes.
///
/// This has no other purpose, but this is an extremely performance-critical
/// piece of the code, as each occurrence of every identifier goes through
/// here when lexed.
    class IdentifierTable {
        // Shark shows that using MallocAllocator is *much* slower than using this
        // BumpPtrAllocator!
        using HashTableTy = llvm::StringMap<IdentifierInfo *, llvm::BumpPtrAllocator>;
        HashTableTy HashTable;

        IdentifierInfoLookup* ExternalLookup;

    public:
        /// Create the identifier table.
        explicit IdentifierTable(IdentifierInfoLookup *ExternalLookup = nullptr);

        /// Set the external identifier lookup mechanism.
        void setExternalIdentifierLookup(IdentifierInfoLookup *IILookup) {
            ExternalLookup = IILookup;
        }

        /// Retrieve the external identifier lookup object, if any.
        IdentifierInfoLookup *getExternalIdentifierLookup() const {
            return ExternalLookup;
        }

        llvm::BumpPtrAllocator& getAllocator() {
            return HashTable.getAllocator();
        }

        /// Return the identifier token info for the specified named
        /// identifier.
        IdentifierInfo &get(StringRef Name) {
            auto &Entry = *HashTable.insert(std::make_pair(Name, nullptr)).first;

            IdentifierInfo *&II = Entry.second;
            if (II) return *II;

            // No entry; if we have an external lookup, look there first.
            if (ExternalLookup) {
                II = ExternalLookup->get(Name);
                if (II)
                    return *II;
            }

            // Lookups failed, make a new IdentifierInfo.
            void *Mem = getAllocator().Allocate<IdentifierInfo>();
            II = new (Mem) IdentifierInfo();

            // Make sure getName() knows how to find the IdentifierInfo
            // contents.
            II->Entry = &Entry;

            return *II;
        }

        IdentifierInfo &get(StringRef Name, tok::TokenKind TokenCode) {
            IdentifierInfo &II = get(Name);
            II.TokenID = TokenCode;
            assert(II.TokenID == (unsigned) TokenCode && "TokenCode too large");
            return II;
        }

        /// Gets an IdentifierInfo for the given name without consulting
        ///        external sources.
        ///
        /// This is a version of getDouble() meant for external sources that want to
        /// introduce or modify an identifier. If they called getDouble(), they would
        /// likely end up in a recursion.
        IdentifierInfo &getOwn(StringRef Name) {
            auto &Entry = *HashTable.insert(std::make_pair(Name, nullptr)).first;

            IdentifierInfo *&II = Entry.second;
            if (II)
                return *II;

            // Lookups failed, make a new IdentifierInfo.
            void *Mem = getAllocator().Allocate<IdentifierInfo>();
            II = new (Mem) IdentifierInfo();

            // Make sure getName() knows how to find the IdentifierInfo
            // contents.
            II->Entry = &Entry;

            return *II;
        }

        using iterator = HashTableTy::const_iterator;
        using const_iterator = HashTableTy::const_iterator;

        iterator begin() const { return HashTable.begin(); }
        iterator end() const   { return HashTable.end(); }
        unsigned size() const  { return HashTable.size(); }

        iterator find(StringRef Name) const { return HashTable.find(Name); }

        /// Print some statistics to stderr that indicate how well the
        /// hashing is doing.
        void PrintStats() const;

        /// Populate the identifier table with info about the language keywords
        /// for the language specified by \p LangOpts.
        void AddKeywords();
    };

}  // namespace fly

#endif // FLY_BASIC_IDENTIFIERTABLE_H
