//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/SymTable.h - AST Context header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYMTABLE_H
#define FLY_SYMTABLE_H

#include <llvm/ADT/StringMap.h>

namespace fly {

    class SymModule;
    class SymNameSpace;

    /**
     * AST Context
     */
    class SymTable {

        friend class SymBuilder;

        llvm::StringMap<SymModule *> Modules;

        // The Default NameSpace
        SymNameSpace *DefaultNameSpace;

        // All NameSpaces in the Context
        llvm::StringMap<SymNameSpace *> NameSpaces;

        SymTable();

    public:

        ~SymTable();

        llvm::StringMap<SymModule *> getModules() const;

        SymNameSpace *getDefaultNameSpace() const;

        llvm::StringMap<SymNameSpace*> getNameSpaces() const;
    };
}

#endif //FLY_SYMTABLE_H
