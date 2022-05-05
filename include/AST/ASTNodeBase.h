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

#include "ASTFunction.h"
#include "Basic/SourceLocation.h"
#include "llvm/ADT/StringMap.h"

namespace fly {

    class ASTContext;
    class ASTGlobalVar;
    class ASTClass;
    class ASTFunction;
    class ASTFunctionCall;
    class ASTUnrefGlobalVar;
    class ASTUnrefCall;

    class ASTNodeBase {

        friend class Sema;
        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        ASTContext* Context;

        // Node FileName
        const std::string Name;

        // Private Global Vars
        llvm::StringMap<ASTGlobalVar *> GlobalVars;

        // Public Functions
        std::unordered_set<ASTFunction*> Functions;

        // Calls created on Functions creations, each Function have a Call defined here
        llvm::StringMap<std::vector<ASTFunctionCall *>> FunctionCalls;

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

        const std::unordered_set<ASTFunction*> &getFunctions() const;

        const llvm::StringMap<ASTClass *> &getClasses() const;

        virtual std::string str() const;

    };
}

#endif //FLY_ASTNODEBASE_H
