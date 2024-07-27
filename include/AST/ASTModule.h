//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTModule.h - AST Module header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_MODULE_H
#define FLY_AST_MODULE_H

#include "llvm/ADT/StringMap.h"

#include <map>

namespace fly {

    class CodeGenModule;
    class ASTContext;
    class ASTNameSpace;
    class ASTImport;
    class ASTGlobalVar;
    class ASTFunction;
    class FileID;
    class ASTType;
    class ASTIdentity;
    class ASTExpr;
    class ASTVarRef;

    class ASTModule {

        friend class Sema;
        friend class SemaResolver;
        friend class SemaBuilder;

        // The Context
        ASTContext* Context = nullptr;

        // Namespace declaration
        ASTNameSpace *NameSpace = nullptr;

        // Module Id
        const uint64_t Id;

        // Module FileName
        const std::string Name;

        // Global Vars
        llvm::SmallVector<ASTGlobalVar *, 8> GlobalVars;

        // Functions
        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> Functions;

        const bool Header;

        // Contains all Imports, the key is Alias or Name
        llvm::SmallVector<ASTImport *, 8> Imports;

        // Contains all Imports, the key is Alias or Name
        llvm::SmallVector<ASTImport *, 8> AliasImports;
        
        // All used GlobalVars
        llvm::StringMap<ASTGlobalVar *> ExternalGlobalVars;
        
        // All invoked Calls
        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> ExternalFunctions;

        llvm::StringMap<ASTIdentity *> ExternalIdentities;

        llvm::SmallVector<ASTIdentity *, 8> Identities;

        CodeGenModule *CodeGen = nullptr;

        ASTModule() = delete;

        ~ASTModule();

        ASTModule(uint64_t &Id, std::string Name, ASTContext *Context, bool isHeader);

    public:

        const uint64_t getId() const;

        bool isHeader() const;

        ASTContext &getContext() const;

        std::string getName();

        ASTNameSpace* getNameSpace();

        const llvm::SmallVector<ASTImport *, 8> &getImports();

        const llvm::SmallVector<ASTImport *, 8> &getAliasImports();

        const llvm::StringMap<ASTGlobalVar *> &getExternalGlobalVars() const;

        const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &getExternalFunctions() const;

        const llvm::SmallVector<ASTIdentity *, 8> &getIdentities() const;

        const llvm::SmallVector<ASTGlobalVar *, 8> &getGlobalVars() const;

        const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &getFunctions() const;

        CodeGenModule *getCodeGen() const;

        void setCodeGen(CodeGenModule *CGM);

        std::string str() const;
    };
}

#endif //FLY_AST_MODULE_H
