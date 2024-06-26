//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNode.h - AST Node
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTNODE_H
#define FLY_ASTNODE_H

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

    class ASTNode {

        friend class Sema;
        friend class SemaResolver;
        friend class SemaBuilder;

        // The Context
        ASTContext* Context = nullptr;

        // Namespace declaration
        ASTNameSpace *NameSpace = nullptr;

        // Node FileName
        const std::string Name;

        // Global Vars
        llvm::StringMap<ASTGlobalVar *> GlobalVars;

        // Functions
        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> Functions;

        const bool Header;

        // Contains all Imports, the key is Alias or Name
        llvm::StringMap<ASTImport *> Imports;

        // Contains all Imports, the key is Alias or Name
        llvm::StringMap<ASTImport *> AliasImports;
        
        // All used GlobalVars
        llvm::StringMap<ASTGlobalVar *> ExternalGlobalVars;
        
        // All invoked Calls
        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> ExternalFunctions;

        ASTIdentity *Identity = nullptr;

        ASTNode() = delete;

        ~ASTNode();

        ASTNode(const std::string FileName, ASTContext *Context, bool isHeader);

    public:

        const bool isHeader() const;

        ASTContext &getContext() const;

        const std::string getName();

        ASTNameSpace* getNameSpace();

        const llvm::StringMap<ASTImport*> &getImports();

        const llvm::StringMap<ASTImport*> &getAliasImports();

        const llvm::StringMap<ASTGlobalVar *> &getExternalGlobalVars() const;

        const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &getExternalFunctions() const;

        ASTIdentity *getIdentity() const;

        const llvm::StringMap<ASTGlobalVar *> &getGlobalVars() const;

        const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &getFunctions() const;

        virtual std::string str() const;
    };
}

#endif //FLY_ASTNODE_H
