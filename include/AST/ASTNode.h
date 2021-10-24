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
    class ASTFunc;
    class ASTClass;
    class FileID;
    class ASTUnrefGlobalVar;
    class ASTUnrefCall;
    class ASTType;
    class ASTExpr;
    class ASTVarRef;

    class ASTNode : public ASTNodeBase {

        friend class ASTResolver;
        friend class ASTContext;
        friend class ASTNameSpace;

        // CodeGen Module
        CodeGenModule *CGM;

        // Namespace declaration
        ASTNameSpace *NameSpace = nullptr;

        // Contains all Imports which will be converted in Dependencies
        llvm::StringMap<ASTImport *> Imports;
        
        // All used GlobalVars
        std::vector<ASTGlobalVar *> ExternalGlobalVars;
        
        // All invoked Calls
        std::vector<ASTFunc *> ExternalFunctions;

    public:
        ASTNode() = delete;

        ~ASTNode();

        ASTNode(const llvm::StringRef &FileName, ASTContext *Context, CodeGenModule * CGM);

        CodeGenModule *getCodeGen() const;

        void setDefaultNameSpace();

        ASTNameSpace *setNameSpace(llvm::StringRef Name);

        ASTNameSpace* getNameSpace();

        ASTNameSpace *FindNameSpace(const StringRef &string);

        bool AddImport(ASTImport *Import);

        const llvm::StringMap<ASTImport*> &getImports();

        bool AddGlobalVar(ASTGlobalVar *GVar);

        bool AddFunction(ASTFunc *Func);

        bool AddClass(ASTClass *Class);

        bool AddExternalGlobalVar(ASTGlobalVar *Var);

        const std::vector<ASTGlobalVar *> &getExternalGlobalVars() const;

        bool AddExternalFunction(ASTFunc *Call);

        const std::vector<ASTFunc *> &getExternalFunctions() const;

        bool AddUnrefCall(ASTFuncCall *Call);

        bool AddUnrefGlobalVar(ASTVarRef *VarRef);

        bool Resolve();
    };
}

#endif //FLY_ASTNODE_H
