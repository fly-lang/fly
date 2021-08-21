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
#include "ASTFunc.h"
#include <unordered_set>

namespace fly {

    class ASTNodeBase;
    class ASTNameSpace;
    class ASTImport;
    class ASTGlobalVar;
    class ASTClass;
    class FileID;

    class ASTNode : public ASTNodeBase {

        friend ASTContext;
        friend ASTNameSpace;

        // CodeGen Module
        CodeGenModule *CGM;

        // Namespace declaration
        ASTNameSpace *NameSpace = nullptr;

        // Contains all Imports which will be converted in Dependencies
        llvm::StringMap<ASTImport *> Imports;

        // Private Global Vars
        llvm::StringMap<ASTGlobalVar *> GlobalVars;

        // Public Functions
        std::unordered_set<ASTFunc*> Functions;

        // Calls into Node resolution
        llvm::StringMap<std::vector<ASTFuncCall *>> ResolvedCalls;

        // Contains all unresolved VarRef with GlobalVar
        std::vector<ASTVarRef *> UnRefGlobalVars;

        // Contains all unresolved Calls with Function
        std::vector<ASTFuncCall *> UnRefCalls;

    public:

        ASTNode() = delete;

        ~ASTNode();

        ASTNode(const llvm::StringRef &FileName, ASTContext *Context, CodeGenModule * CGM);

        CodeGenModule *getCodeGen() const;

        void setDefaultNameSpace();
        ASTNameSpace *setNameSpace(llvm::StringRef NS);
        ASTNameSpace* getNameSpace();
        ASTNameSpace *findNameSpace(const StringRef &string);

        bool addImport(ASTImport *NewImport);
        const llvm::StringMap<ASTImport*> &getImports();

        bool addGlobalVar(ASTGlobalVar *Var);
        const llvm::StringMap<ASTGlobalVar *> &getGlobalVars();

        bool addFunction(ASTFunc *Func);
        const std::unordered_set<ASTFunc*> &getFunctions() const;

        bool addResolvedCall(ASTFuncCall *Call);
        const llvm::StringMap<std::vector<ASTFuncCall *>> &getResolvedCalls() const;

        bool addClass(ASTClass *Class);
        const llvm::StringMap<ASTClass *> &getClasses();

        static ASTType *ResolveExprType(ASTExpr *E);

        void addUnRefCall(ASTFuncCall *Call);

        void addUnRefGlobalVar(ASTVarRef *Var);

        bool Resolve();

        bool Resolve(std::vector<ASTVarRef *> &UnRefGlobalVars,
                              llvm::StringMap<ASTGlobalVar *> &GlobalVars,
                              std::vector<ASTFuncCall *> &UnRefCalls,
                              llvm::StringMap<std::vector<ASTFuncCall *>> &ResolvedCalls);

        bool ResolveGlobalVar(std::vector<ASTVarRef *> &UnRefGlobalVars,
                             llvm::StringMap<ASTGlobalVar *> &GlobalVars);

        bool ResolveFunction(ASTFunc *Function,
                     std::vector<ASTFuncCall *> &UnRefCalls,
                     llvm::StringMap<std::vector<ASTFuncCall *>> &ResolvedCalls);
    };
}

#endif //FLY_ASTNODE_H
