//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNameSpace.h - AST Namespace header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_NAMESPACE_H
#define FLY_AST_NAMESPACE_H

#include "ASTIdentifier.h"

#include "llvm/ADT/StringMap.h"

#include <map>

namespace fly {

    class ASTModule;
    class ASTContext;
    class ASTGlobalVar;
    class ASTIdentity;
    class ASTFunction;
    class ASTImport;
    class ASTIdentityType;

    class ASTNameSpace : public ASTIdentifier {

        friend class Sema;
        friend class SemaResolver;
        friend class SemaBuilder;

        // The Context
        ASTContext* Context = nullptr;

        // AST by FileID
        llvm::StringMap<ASTModule *> Modules;

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

        // Identity Types
        llvm::StringMap<ASTIdentityType *> IdentityTypes;

        ASTNameSpace(const SourceLocation &Loc, llvm::StringRef Name, ASTContext *Context, bool ExternalLib = false);

    public:

        ~ASTNameSpace();

        static const std::string DEFAULT;

        const llvm::StringMap<ASTModule *> &getModules() const;

        bool isExternalLib() const;

        const llvm::StringMap<ASTIdentity *> &getIdentities() const;

        const llvm::StringMap<ASTGlobalVar *> &getGlobalVars() const;

        const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &getFunctions() const;

        const llvm::StringMap<ASTIdentityType *> &getIdentityTypes() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_NAMESPACE_H
