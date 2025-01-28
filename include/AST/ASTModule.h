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

    class ASTModule
    {
        friend class Sema;
        friend class SemaResolver;
        friend class SemaBuilder;

        // The Context
        ASTContext* Context = nullptr;

        // Namespace declaration
        llvm::SmallVector<ASTNameSpace*, 8> NameSpaces;

        // Module Id
        const uint64_t Id;

        // Module FileName
        const std::string Name;

        // Module is Header
        const bool Header;

        // All Top Definitions sorted in the order it appears in the code: NameSpace, Imports, Global Vars, Functions
        llvm::SmallVector<ASTBase *, 8> Definitions;

        // Contains all Imports, the key is Alias or Name
        llvm::SmallVector<ASTImport*, 8> Imports;

        // Contains all Imports, the key is Alias or Name
        llvm::SmallVector<ASTImport*, 8> AliasImports;

        // Global Vars
        llvm::SmallVector<ASTGlobalVar*, 8> GlobalVars;

        // Functions
        llvm::SmallVector<ASTFunction*, 8> Functions;

        // Identities: Class, Structure, Enum
        llvm::SmallVector<ASTIdentity*, 8> Identities;

        CodeGenModule* CodeGen = nullptr;

        ASTModule() = delete;

        ~ASTModule();

        ASTModule(uint64_t& Id, std::string Name, ASTContext* Context, bool isHeader);

    public:
        const uint64_t getId() const;

        bool isHeader() const;

        ASTContext& getContext() const;

        std::string getName();

        llvm::SmallVector<ASTNameSpace*, 8> getNameSpaces();

        ASTNameSpace* getNameSpace();

        const llvm::SmallVector<ASTImport*, 8>& getImports();

        const llvm::SmallVector<ASTImport*, 8>& getAliasImports();

        const llvm::SmallVector<ASTIdentity*, 8>& getIdentities() const;

        const llvm::SmallVector<ASTGlobalVar*, 8>& getGlobalVars() const;

        const llvm::SmallVector<ASTFunction*, 8>& getFunctions() const;

        CodeGenModule* getCodeGen() const;

        void setCodeGen(CodeGenModule* CGM);

        std::string str() const;
    };
}

#endif //FLY_AST_MODULE_H
