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

#include "ASTNodeBase.h"

namespace fly {

    class CodeGenModule;
    class ASTNodeBase;
    class ASTNameSpace;
    class ASTImport;
    class ASTGlobalVar;
    class ASTFunction;
    class ASTClass;
    class FileID;
    class ASTUnrefGlobalVar;
    class ASTUnrefFunctionCall;
    class ASTType;
    class ASTExpr;
    class ASTVarRef;

    class ASTNode : public ASTNodeBase {

        friend class Sema;
        friend class SemaResolver;
        friend class SemaBuilder;

        // Namespace declaration
        ASTNameSpace *NameSpace = nullptr;

        const bool Header;

        // Contains all Imports, the key is Alias or Name
        llvm::StringMap<ASTImport *> Imports;
        
        // All used GlobalVars
        llvm::StringMap<ASTGlobalVar *> ExternalGlobalVars;
        
        // All invoked Calls
        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> ExternalFunctions;

        ASTClass *Class = nullptr;

        ASTNode() = delete;

        ~ASTNode();

        ASTNode(const std::string FileName, ASTContext *Context, bool isHeader);

    public:

        const bool isHeader() const;

        ASTNameSpace* getNameSpace();

        ASTImport *FindImport(const llvm::StringRef string);

        const llvm::StringMap<ASTImport*> &getImports();

        const llvm::StringMap<ASTGlobalVar *> &getExternalGlobalVars() const;

        const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &getExternalFunctions() const;

        ASTClass *getClass() const;

        virtual std::string str() const;
    };
}

#endif //FLY_ASTNODE_H
