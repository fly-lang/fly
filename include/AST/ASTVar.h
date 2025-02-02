//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.h - AST Var header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_VAR_H
#define FLY_AST_VAR_H

#include "ASTBase.h"
#include "ASTExpr.h"

namespace fly {

    class SourceLocation;
    class CodeGenVarBase;
    class ASTAssignmentStmt;
    class ASTScope;

    /**
     * Base Var used in:
     *  - LocalVar
     *  - GlobalVar
     */
    class ASTVar : public ASTBase {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTTypeRef *TypeRef;

        llvm::StringRef Name;

        SmallVector<ASTScope *, 8> Scopes;

        ASTExpr *Expr;

        ASTAssignmentStmt *Initialization = nullptr;

    protected:

        ASTVar(const SourceLocation &Loc, ASTTypeRef *Type, llvm::StringRef Name, SmallVector<ASTScope *, 8> &Scopes);

    public:

        ASTTypeRef *getTypeRef() const;

        llvm::StringRef getName() const;

        bool isConstant() const;

        bool isInitialized();

        ASTAssignmentStmt *getInitialization();

        void setInitialization(ASTAssignmentStmt *VarDefine);

        const SmallVector<ASTScope *, 8> &getScopes() const;

        ASTExpr *getExpr() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_VAR_H
