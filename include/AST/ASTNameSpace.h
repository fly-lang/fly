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

#include "ASTIdentifier.h"

#include "llvm/ADT/StringMap.h"

#include <map>

namespace fly {

    class ASTNode;
    class ASTContext;
    class ASTGlobalVar;
    class ASTIdentity;
    class ASTFunction;
    class ASTImport;

    class ASTNameSpace : public ASTIdentifier {

        friend class Sema;
        friend class SemaResolver;
        friend class SemaBuilder;
        friend class Sys;

        // The Context
        ASTContext* Context = nullptr;

        // AST by FileID
        llvm::StringMap<ASTNode *> Nodes;

        // Contains all Imports, the key is Alias or Name
        llvm::StringMap<ASTImport *> AliasImports;

        // All used GlobalVars
        llvm::StringMap<ASTGlobalVar *> ExternalGlobalVars;

        bool ExternalLib;

        // Global Vars
        llvm::StringMap<ASTGlobalVar *> GlobalVars;

        // Functions
        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> Functions;

        // Classes or Enums
        llvm::StringMap<ASTIdentity *> Identities;

        ASTNameSpace(const SourceLocation &Loc, llvm::StringRef Name, ASTContext *Context, bool ExternalLib = false);

    public:

        ~ASTNameSpace();

        static const std::string DEFAULT;

        const llvm::StringMap<ASTNode *> &getNodes() const;

        bool isExternalLib() const;

        const llvm::StringMap<ASTIdentity *> &getIdentities() const;

        const llvm::StringMap<ASTGlobalVar *> &getGlobalVars() const;

        const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &getFunctions() const;

        std::string print() const;

        virtual std::string str() const;
    };
}

#endif //FLY_ASTNAMESPACE_H
