//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNode.h - AST Unit
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
#include "VarExpr.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "vector"

namespace fly {

    class ASTNodeBase;
    class ASTNameSpace;
    class FileID;

    class ASTNode : public ASTNodeBase {

        friend ASTContext;
        friend ASTNameSpace;

        // Namespace declaration
        ASTNameSpace *NameSpace;

        // Contains all Imports which will be converted in Dependencies
        llvm::StringMap<ImportDecl *> Imports;

        // Private Global Vars
        llvm::StringMap<GlobalVarDecl *> Vars;

        bool FirstNode;

    public:

        ASTNode() = delete;

        ASTNode(const StringRef &fileName, const FileID &fid, ASTContext *Context);

        bool isFirstNode() const;
        void setFirstNode(bool firstNode);

        void setNameSpace();
        void setNameSpace(const StringRef &NS);
        const ASTNameSpace* getNameSpace();

        bool addImport(StringRef Name, StringRef Alias = "");
        const llvm::StringMap<ImportDecl*> &getImports();

        const llvm::StringMap<GlobalVarDecl *> &getVars();

        bool Finalize();

        GlobalVarDecl *addIntVar(VisibilityKind Visibility, ModifiableKind Modifiable, StringRef Name,
                                 int *Val = nullptr);
        GlobalVarDecl *addFloatVar(VisibilityKind Visibility, ModifiableKind Modifiable, StringRef Name,
                                   float *Val = nullptr);
        GlobalVarDecl *addBoolVar(VisibilityKind Visibility, ModifiableKind Modifiable, StringRef Name,
                                  bool *Val = nullptr);

    private:
        bool addVar(GlobalVarDecl *Var);
    };
}

#endif //FLY_ASTNODE_H
