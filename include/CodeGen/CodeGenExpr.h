//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGExpr.h - Code Generator of Expression
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGENEXPR_H
#define FLY_CODEGENEXPR_H

#include "AST/ASTExpr.h"

namespace fly {

    class CodeGenModule;
    class ASTOperatorExpr;

    class VirtualExpr : public ASTExpr {

        llvm::Value *Val;

    public:
        VirtualExpr(llvm::Value *Val) : Val(Val), ASTExpr(SourceLocation()) {}

        ExprKind getKind() const override {
            return ExprKind::EXPR_VIRTUAL;
        }

        ASTType *getType() const {
            return nullptr;
        }

        Value *getVal() const {
            return Val;
        }
    };

    class CodeGenExpr {

        CodeGenModule * CGM;

        const ASTType *Type;

        llvm::Value *Val;

        std::vector<llvm::Value *> PostIncrements;

    public:
        CodeGenExpr(CodeGenModule *CGM, ASTExpr *Expr, const ASTType *Type);

        llvm::Value *getValue() const;

        llvm::Value *Generate(ASTExpr *Origin);

        llvm::Value *GenValue(ASTExpr *Origin);

        void GenIncDec(ASTGroupExpr *Group);

        llvm::Value *GenGroup(ASTGroupExpr *Origin, ASTGroupExpr *New, int Idx, ASTExpr *E1 = nullptr,
                              ASTOperatorExpr * OP1 = nullptr);

        llvm::Value *GenOperation(ASTExpr *E1, ASTOperatorExpr *Op, ASTExpr *E2);

        bool hasOpPrecedence(ASTExpr *OP);

        bool canIterate(int Idx, ASTGroupExpr *Group);
    };
}


#endif //FLY_CODEGENEXPR_H
