//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SymbolTable.h - symbol table
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_TABLE_H
#define FLY_SYM_TABLE_H

#include "Symbol.h"
#include <AST/ASTType.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/SmallVector.h>

namespace fly {

    class SemaModule;
    class SemaNameSpace;
    class SemaType;

    /**
     * Symbol Table
     * Manages symbols in a hierarchical scope structure with parent-child relationships
     */
    class SymbolTable {

        llvm::StringMap<llvm::SmallVector<Symbol *, 8>> Table;

        SymbolTable* Parent;

        llvm::SmallVector<SymbolTable*, 8> Children;

    public:

        SymbolTable(SymbolTable* Parent = nullptr);

        ~SymbolTable();

        bool insert(Symbol *Sym);

    	void addChild(SymbolTable *Child);

    	size_t size() const;

    	llvm::SmallVector<Symbol *, 8> *lookup(llvm::StringRef Name);

        llvm::SmallVector<Symbol *, 8> *lookupInParents(llvm::StringRef Name);

        const llvm::StringMap<llvm::SmallVector<Symbol *, 8>> &getAll() const;

        llvm::SmallVector<Symbol *, 8> *lookupInChildren(llvm::StringRef Name);

        SymbolTable* pushScope();

        SymbolTable* getParent();

    	llvm::SmallVector<SymbolTable*, 8> getChildren();

        /**
         * Delete all child scopes created via pushScope()
         * This should be called before destroying a SymbolTable if you want
         * to clean up all descendant scopes recursively
         */
        void deleteChildren();

    };
}

#endif //FLY_SYM_TABLE_H
