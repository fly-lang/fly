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

#include "AST/FuncDecl.h"
#include "llvm/ADT/StringMap.h"
#include <unordered_set>

namespace fly {

    class ASTNode;
    class ASTContext;
    class GlobalVarDecl;
    class ClassDecl;

    class ASTNameSpace {

        friend ASTContext;
        friend ASTNode;

        llvm::StringRef NameSpace;

        // AST by FileID
        llvm::StringMap<ASTNode *> Nodes;

        // Public Global Vars
        llvm::StringMap<GlobalVarDecl *> GlobalVars;

        // Public Functions
        std::unordered_set<FuncDecl *> Functions;

        // Calls into NameSpace resolution
        std::unordered_set<FuncCall *, FuncCallHash, FuncCallComp> Calls;

        // Public Classes
        llvm::StringMap<ClassDecl *> Classes;

    public:
        ASTNameSpace(const llvm::StringRef &NS);

        ~ASTNameSpace();

        static const llvm::StringRef DEFAULT;

        const llvm::StringRef &getNameSpace() const;

        const llvm::StringMap<ASTNode *> &getNodes() const;

        const llvm::StringMap<GlobalVarDecl *> &getGlobalVars() const;

        const std::unordered_set<FuncDecl *> &getFunctions() const;

        const std::unordered_set<FuncCall *, FuncCallHash, FuncCallComp> &getCalls() const;
        bool addCall(FuncCall *Call);

        const llvm::StringMap<ClassDecl *> &getClasses() const;

        bool Finalize();
    };
}

#endif //FLY_ASTNAMESPACE_H
