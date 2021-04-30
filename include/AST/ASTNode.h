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
#include "ASTNameSpace.h"
#include "ImportDecl.h"
#include "GlobalVarDecl.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "FunctionDecl.h"
#include "ClassDecl.h"

namespace fly {

    class ASTNodeBase;
    class ASTNameSpace;
    class FileID;

    class ASTNode : public ASTNodeBase {

        friend ASTContext;
        friend ASTNameSpace;

        // Namespace declaration
        ASTNameSpace *NameSpace = NULL;

        // Contains all Imports which will be converted in Dependencies
        llvm::StringMap<ImportDecl *> Imports;

        // Private Global Vars
        llvm::StringMap<GlobalVarDecl *> Vars;

        // Private Functions
        llvm::StringMap<FunctionDecl *> Functions;

        // Private Classes
        llvm::StringMap<ClassDecl *> Classes;

        bool FirstNode;

    public:

        ASTNode() = delete;

        ASTNode(const StringRef fileName, const FileID &fid, ASTContext *Context);

        ~ASTNode();

        bool isFirstNode() const;
        void setFirstNode(bool firstNode);

        void setNameSpace();
        void setNameSpace(StringRef NS);
        const ASTNameSpace* getNameSpace();

        bool addImport(ImportDecl *NewImport);
        const llvm::StringMap<ImportDecl*> &getImports();

        bool addGlobalVar(GlobalVarDecl *Var);
        const llvm::StringMap<GlobalVarDecl *> &getVars();

        bool addFunction(FunctionDecl *Func);
        const llvm::StringMap<FunctionDecl *> &getFunctions();

        bool addClass(ClassDecl *Class);
        const llvm::StringMap<ClassDecl *> &getClasses();

        bool Finalize();
    };
}

#endif //FLY_ASTNODE_H
