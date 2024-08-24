//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/SwitchBlock.h - AST Switch Block Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_SWITCHBLOCK_H
#define FLY_AST_SWITCHBLOCK_H

#include "ASTStmt.h"

namespace fly {

    class ASTSwitchCase;
    class ASTValueExpr;
    class ASTType;
    class ASTVarRef;

    class ASTSwitchStmt : public ASTStmt {

        friend class SemaBuilder;
        friend class SemaBuilderSwitchStmt;
        friend class SemaResolver;
        friend class SemaValidator;

        // The Switch Expression
        ASTVarRef *VarRef = nullptr;

        // The Case Blocks
        llvm::SmallVector<ASTSwitchCase *, 8> Cases;

        // The Default Block
        ASTStmt *Default = nullptr;

        explicit ASTSwitchStmt(const SourceLocation &Loc);

    public:

        ASTVarRef *getVarRef() const;

        llvm::SmallVector<ASTSwitchCase *, 8> &getCases();

        ASTStmt *getDefault();

        std::string str() const override;
    };

    class ASTSwitchCase {

        friend class SemaBuilder;
        friend class SemaBuilderSwitchStmt;
        friend class SemaResolver;

        // The case value
        ASTValueExpr *Value = nullptr;

        // The Case Block
        ASTStmt *Stmt = nullptr;

        explicit ASTSwitchCase(const SourceLocation &Loc);

    public:

        ASTValueExpr *getValueExpr();

        ASTStmt *getStmt() const;

        std::string str() const;
    };
}


#endif //FLY_AST_SWITCHBLOCK_H
