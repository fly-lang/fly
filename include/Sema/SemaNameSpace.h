//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/SemaNameSpace.h - AST Namespace header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_NAMESPACE_H
#define FLY_SEMA_NAMESPACE_H

#include <unordered_map>

#include "Sema/SemaNode.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"

namespace fly {

    class ASTModule;
    class ASTNameSpace;
    class SemaGlobalVar;
    class SemaFunction;
    class SemaClassType;
    class SemaEnumType;
    class CodeGenModule;
    class SemaType;
    class SymbolTable;

    class SemaNameSpace : public SemaNode {

        SymbolTable *Symbols;

        llvm::StringRef Name;

        SemaNameSpace *Parent;

        llvm::StringMap<SemaNameSpace *> Children;

        llvm::SmallVector<ASTModule *, 8> Modules;

        // Global Vars
        llvm::StringMap<SemaGlobalVar *> GlobalVars;

        // Functions
        llvm::StringMap<SemaFunction *> Functions;

        // Types
        llvm::StringMap<SemaType *> Types;

    public:

        explicit SemaNameSpace(llvm::StringRef Name, SemaNameSpace *Parent = nullptr);

        SymbolTable *getSymbols() const;

        ~SemaNameSpace();

        llvm::StringRef getName() const;

        SemaNameSpace *getParent() const;

        const llvm::StringMap<SemaNameSpace *> &getChildren() const;

        const llvm::SmallVector<ASTModule *, 8> &getModules() const;

        const llvm::StringMap<SemaGlobalVar *> &getGlobalVars() const;

        const llvm::StringMap<SemaFunction *> &getFunctions() const;

        const llvm::StringMap<SemaType *> &getTypes() const;

    };
}

#endif //FLY_SEMA_NAMESPACE_H
