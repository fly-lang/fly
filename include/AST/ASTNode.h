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
#include "llvm/ADT/StringMap.h"
#include <unordered_set>

namespace fly {

    class ASTNodeBase;
    class ASTNameSpace;
    class ImportDecl;
    class GlobalVarDecl;
    class FuncDecl;
    class FuncCall;
    class ClassDecl;
    class FileID;

    class ASTNode : public ASTNodeBase {

        friend ASTContext;
        friend ASTNameSpace;

        // Namespace declaration
        ASTNameSpace *NameSpace = NULL;

        // Contains all Imports which will be converted in Dependencies
        llvm::StringMap<ImportDecl *> Imports;

        // Private Global Vars
        llvm::StringMap<GlobalVarDecl *> GlobalVars;

        // Private Functions
        std::unordered_set<FuncDecl *> Functions;

        // Private Classes
        llvm::StringMap<ClassDecl *> Classes;

        bool FirstNode;

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
        const llvm::StringMap<GlobalVarDecl *> &getGlobalVars();

        bool addFunction(FuncDecl *Func);
        const std::unordered_set<FuncDecl *> &getFunctions();

        bool addClass(ClassDecl *Class);
        const llvm::StringMap<ClassDecl *> &getClasses();

        bool Finalize();
    };
}

#endif //FLY_ASTNODE_H
