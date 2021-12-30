//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNodeBase.h - Base AST Node
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTNODEBASE_H
#define FLY_ASTNODEBASE_H

#include "ASTFunc.h"
#include "Basic/SourceLocation.h"
#include "llvm/ADT/StringMap.h"

namespace fly {

    class ASTContext;
    class ASTGlobalVar;
    class ASTClass;
    class ASTFunc;
    class ASTFuncCall;
    class ASTUnrefGlobalVar;
    class ASTUnrefCall;

    class ASTNodeBase {

    protected:

        ASTContext* Context;

        // Node FileName
        const std::string Name;

        // Private Global Vars
        llvm::StringMap<ASTGlobalVar *> GlobalVars;

        // Public Functions
        std::unordered_set<ASTFunc*> Functions;

        // Calls created on Functions creations, each Function have a Call defined here
        llvm::StringMap<std::vector<ASTFuncCall *>> FunctionCalls;

        // Public Classes
        llvm::StringMap<ASTClass *> Classes;

        // Contains all unresolved VarRef to a GlobalVar
        std::vector<ASTUnrefGlobalVar *>  UnrefGlobalVars;

        // Contains all unresolved Function Calls
        std::vector<ASTUnrefCall *> UnrefFunctionCalls;

    public:

        ASTNodeBase() = delete;

        ASTNodeBase(const std::string &Name, ASTContext* Context);

        const std::string& getName();

        ASTContext &getContext() const;

        const llvm::StringMap<ASTGlobalVar *> &getGlobalVars() const;

        const std::unordered_set<ASTFunc*> &getFunctions() const;

        const llvm::StringMap<std::vector<ASTFuncCall *>> &getFunctionCalls() const;

        const llvm::StringMap<ASTClass *> &getClasses() const;

        bool AddFunctionCall(ASTFuncCall *Call);

        virtual std::string str() const;

    };
}

#endif //FLY_ASTNODEBASE_H
