//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/SymbolTable.h - AST Context header
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
#include <vector>

namespace fly {

    class SemaModule;
    class SemaNameSpace;
    class SemaType;

    /**
     * Symbol Table
     * Manages symbols in a hierarchical scope structure with parent-child relationships
     */
    class SymbolTable {

        llvm::StringMap<Symbol*> Table;

        SymbolTable* Parent;

        std::vector<SymbolTable*> Children;

    public:

        SymbolTable(SymbolTable* Parent = nullptr);

        ~SymbolTable();

        void insert(Symbol* Sym);

        Symbol* lookup(llvm::StringRef Name);

        SymbolTable* pushScope();

        SymbolTable* getParent();

        /**
         * Delete all child scopes created via pushScope()
         * This should be called before destroying a SymbolTable if you want
         * to clean up all descendant scopes recursively
         */
        void deleteChildren();

    };
}

#endif //FLY_SYM_TABLE_H
