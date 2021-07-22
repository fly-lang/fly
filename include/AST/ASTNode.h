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

        // Namespace declaration
        ASTNameSpace *NameSpace = NULL;

        // Contains all Imports which will be converted in Dependencies
        llvm::StringMap<ASTImport *> Imports;

        // true if this is the first node Finalized in the Context
        bool FirstNode;

        // Private Global Vars
        llvm::StringMap<ASTGlobalVar *> GlobalVars;

        // Public Functions
        std::unordered_set<ASTFunc*> Functions;

        // Calls into Node resolution
        llvm::StringMap<std::vector<ASTFuncCall *>> ResolvedCalls;

    public:

        ASTNode() = delete;

        ~ASTNode();

        ASTNode(const llvm::StringRef &FileName, const FileID &FID, ASTContext *Context);

        bool isFirstNode() const;
        void setFirstNode(bool firstNode);

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

        bool Finalize();
    };
}

#endif //FLY_ASTNODE_H
