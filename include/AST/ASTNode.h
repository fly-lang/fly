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
#include "FuncDecl.h"
#include "llvm/ADT/StringMap.h"
#include <unordered_set>

namespace fly {

    class ASTNodeBase;
    class ASTNameSpace;
    class ImportDecl;
    class GlobalVarDecl;
    class ClassDecl;
    class FileID;

    class ASTNode : public ASTNodeBase {

        friend ASTContext;
        friend ASTNameSpace;

        // Namespace declaration
        ASTNameSpace *NameSpace = NULL;

        // Contains all Imports which will be converted in Dependencies
        llvm::StringMap<ImportDecl *> Imports;

        // true if this is the first node Finalized in the Context
        bool FirstNode;

        // Private Global Vars
        llvm::StringMap<GlobalVarDecl *> GlobalVars;

        // Public Functions
        std::unordered_set<FuncDecl *, FuncDeclHash, FuncDeclComp> Functions;

        // Calls into Node resolution
        llvm::StringMap<std::vector<FuncCall *>> ResolvedCalls;

    public:

        ASTNode() = delete;

        ~ASTNode();

        ASTNode(const llvm::StringRef &fileName, const FileID &fid, ASTContext *Context);

        bool isFirstNode() const;
        void setFirstNode(bool firstNode);

        void setDefaultNameSpace();
        ASTNameSpace *setNameSpace(llvm::StringRef NS);
        ASTNameSpace* getNameSpace();
        ASTNameSpace *findNameSpace(const StringRef &string);

        bool addImport(ImportDecl *NewImport);
        const llvm::StringMap<ImportDecl*> &getImports();

        bool addGlobalVar(GlobalVarDecl *Var);
        const std::vector<GlobalVarDecl *> getGlobalVars();

        bool addFunction(FuncDecl *Func);
        const std::unordered_set<FuncDecl *, FuncDeclHash, FuncDeclComp> getFunctions() const;

        bool addResolvedCall(FuncCall *Call);
        const llvm::StringMap<std::vector<FuncCall *>> &getResolvedCalls() const;

        bool addClass(ClassDecl *Class);
        const llvm::StringMap<ClassDecl *> &getClasses();

        static TypeBase *ResolveExprType(Expr *E);

        bool Finalize();
    };
}

#endif //FLY_ASTNODE_H
