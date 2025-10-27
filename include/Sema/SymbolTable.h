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
#include <unordered_map>
#include <AST/ASTTypeRef.h>

namespace fly {

    class SemaModule;
    class SemaNameSpace;
    class SemaType;

    /**
     * AST Context
     */
    class SymbolTable {

        std::unordered_map<std::string, Symbol*> Table;

        SymbolTable* Parent;

    public:

        SymbolTable(SymbolTable* Parent = nullptr);

        ~SymbolTable();

        void insert(Symbol* Sym);

        Symbol* lookup(const std::string &Name);

        SymbolTable* pushScope();

        SymbolTable* getParent();

    };
}

#endif //FLY_SYM_TABLE_H
