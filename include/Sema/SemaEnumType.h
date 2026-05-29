//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaEnumType.h - enum type semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_ENUM_TYPE_H
#define FLY_SEMA_ENUM_TYPE_H

#include "SemaVisibilityKind.h"
#include "Sema/SemaType.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"

namespace fly {

    class SemaContext;
    class ASTEnum;
    class SemaComment;
    class SemaEnumEntry;
    class SemaModule;
    class SymbolTable;
    enum class SemaVisibilityKind;

    class SemaEnumType : public SemaType {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

        ASTEnum &AST;

        SemaModule *Module = nullptr;

        // Symbol Table
        SymbolTable *Symbols;

        // Nodes: Enum Entries
        llvm::SmallVector<SemaNode *, 8> Nodes;

        // Super Enums
        llvm::StringMap<SemaEnumType *> SuperEnums;

        SemaVisibilityKind Visibility = SemaVisibilityKind::DEFAULT;

        bool Constant;

        // Enum Entries
        llvm::StringMap<SemaEnumEntry *> Entries;

        SemaComment *Comment = nullptr;

        explicit SemaEnumType(ASTEnum &AST, SymbolTable *Symbols);

    public:

        ~SemaEnumType() override;

        ASTEnum &getAST();

        SemaModule *getModule() const;

        SymbolTable *getSymbols() const;

        llvm::SmallVector<SemaNode *, 8> &getNodes();

        const llvm::StringMap<SemaEnumType *> &getBaseEnums() const;

        SemaVisibilityKind getVisibility() const;

        bool isConstant() const;

        const llvm::StringMap<SemaEnumEntry *> &getEntries() const;

        SemaEnumEntry *LookupEntry(llvm::StringRef Name) const;

        void addEntry(SemaEnumEntry *Entry);

        SemaComment *getComment() const;

        bool isDerivedOrEquals(const SemaEnumType *BaseEnumType) const;

        bool isDerived(const SemaEnumType *BaseEnumType) const;

        bool isBaseOrEquals(const SemaEnumType *Derived) const;

        bool isBase(const SemaEnumType *Derived) const;

        std::string str() const override;

        void accept(SemaVisitor& Visitor) override;

    };

}  // end namespace fly

#endif // FLY_SEMA_ENUM_TYPE_H