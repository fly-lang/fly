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

namespace fly {

    class SemaModule;
    class SemaNameSpace;
    class SemaType;

    /**
     * AST Context
     */
    class SymbolTable {

        llvm::StringMap<Symbol*> Table;

        SymbolTable* Parent;

    public:

        SymbolTable(SymbolTable* Parent = nullptr);

        ~SymbolTable();

        void insert(Symbol* Sym);

        Symbol* lookup(llvm::StringRef Name);

        SymbolTable* pushScope();

        SymbolTable* getParent();

    };
}

#endif //FLY_SYM_TABLE_H
