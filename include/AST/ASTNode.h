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
    class ASTUnrefCall;
    class ASTType;
    class ASTExpr;
    class ASTVarRef;

    class ASTNode : public ASTNodeBase {

        friend class Sema;
        friend class SemaResolver;
        friend class SemaBuilder;

        // CodeGen Module
        CodeGenModule *CGM;

        // Namespace declaration
        ASTNameSpace *NameSpace = nullptr;

        const bool Header;

        // Contains all Imports, the key is Alias or Name
        llvm::StringMap<ASTImport *> Imports;
        
        // All used GlobalVars
        llvm::StringMap<ASTGlobalVar *> ExternalGlobalVars;
        
        // All invoked Calls
        std::unordered_set<ASTFunction *> ExternalFunctions;

    public:
        ASTNode() = delete;

        ~ASTNode();

        ASTNode(const std::string FileName, ASTContext *Context);

        ASTNode(const std::string FileName, ASTContext *Context, CodeGenModule * CGM);

        CodeGenModule *getCodeGen() const;

        const bool isHeader() const;

        void setNameSpace(std::string Name);

        ASTNameSpace* getNameSpace();

        void setDefaultNameSpace();

        ASTImport *FindImport(const std::string &string);

        bool AddImport(ASTImport *Import);

        const llvm::StringMap<ASTImport*> &getImports();

        bool AddGlobalVar(ASTGlobalVar *GVar);

        bool AddFunction(ASTFunction *Func);

        bool AddClass(ASTClass *Class);

        bool AddExternalGlobalVar(ASTGlobalVar *Var);

        const llvm::StringMap<ASTGlobalVar *> &getExternalGlobalVars() const;

        bool AddExternalFunction(ASTFunction *Call);

        const std::unordered_set<ASTFunction *> &getExternalFunctions() const;

        bool AddUnrefCall(ASTFunctionCall *Call);

        bool AddUnrefGlobalVar(ASTVarRef *VarRef);
    };
}

#endif //FLY_ASTNODE_H
