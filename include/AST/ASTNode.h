//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTUnit.h - AST Unit
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
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include "vector"

namespace fly {

    class ASTNodeBase;

    class FileID;

    class ASTNode : public ASTNodeBase {

        // Namespace declaration
        ASTNameSpace* NameSpace;

        // Contains all Imports which will be converted in Dependencies
        llvm::StringMap<ImportDecl*> Imports;

        // Private Global Vars
        llvm::StringMap<GlobalVarDecl *> Vars;

    public:

        ASTNode() = delete;

        ASTNode(const StringRef &fileName, const FileID &fid, ASTContext *Context);

        void setNameSpace(const StringRef &NS);
        const ASTNameSpace* getNameSpace();

        bool addImport(StringRef Import);
        const llvm::StringMap<ImportDecl*> &getImports();

        bool addVar(GlobalVarDecl* Var);
        const llvm::StringMap<GlobalVarDecl *> &getVars();

        bool Finalize();
    };
}

#endif //FLY_ASTNODE_H
