//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNameSpace.h - AST Namespace
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTNAMESPACE_H
#define FLY_ASTNAMESPACE_H

#include "AST/ASTFunc.h"
#include "llvm/ADT/StringMap.h"
#include <unordered_set>

namespace fly {

    class ASTNode;
    class ASTContext;
    class ASTGlobalVar;
    class ASTClass;

    class ASTNameSpace {

        friend ASTContext;

        llvm::StringRef Name;

        // AST by FileID
        llvm::StringMap<ASTNode *> Nodes;

        // Public & Default Global Vars
        llvm::StringMap<ASTGlobalVar *> GlobalVars;

        // Public & Default Functions
        std::unordered_set<ASTFunc*> Functions;

        // Calls into NameSpace resolution
        llvm::StringMap<std::vector<ASTFuncCall *>> ResolvedCalls;

        // Contains all unresolved VarRef with GlobalVar
        std::vector<ASTVarRef *> UnRefGlobalVars;

        // Contains all unresolved Calls with Function
        std::vector<ASTFuncCall *> UnRefCalls;

        // Public Classes
        llvm::StringMap<ASTClass *> Classes;

    public:
        ASTNameSpace(const llvm::StringRef &NS);

        ~ASTNameSpace();

        static const llvm::StringRef DEFAULT;

        const llvm::StringRef &getName() const;

        const llvm::StringMap<ASTNode *> &getNodes() const;

        const llvm::StringMap<ASTGlobalVar *> &getGlobalVars() const;
        bool addGlobalVar(ASTGlobalVar *GVar);

        const std::unordered_set<ASTFunc*> &getFunctions() const;
        bool addFunction(ASTFunc *Func);

        const llvm::StringMap<std::vector<ASTFuncCall *>> &getResolvedCalls() const;
        bool addResolvedCall(ASTFuncCall *Call);

        const llvm::StringMap<ASTClass *> &getClasses() const;
        bool addClass(ASTClass *Class);

        void addUnRefCall(ASTFuncCall *Call);

        void addUnRefGlobalVar(ASTVarRef *Var);

        bool Resolve();
    };
}

#endif //FLY_ASTNAMESPACE_H
