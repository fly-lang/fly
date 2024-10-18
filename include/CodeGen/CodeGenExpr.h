//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGExpr.h - Code Generator of Expression
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_EXPR_H
#define FLY_CODEGEN_EXPR_H

namespace fly {

    class CodeGenModule;
    class ASTUnaryOpExpr;
    class ASTBinaryOpExpr;
    class ASTTernaryOpExpr;
    enum class ASTBinaryOpExprKind;

    class CodeGenExpr {

        CodeGenModule * CGM = nullptr;

        llvm::Value *Val = nullptr;

        ASTVar *Var = nullptr;

    public:

        CodeGenExpr(CodeGenModule *CGM, ASTExpr *Expr);

        llvm::Value *GenValue(const ASTExpr *Expr);

        llvm::Value *getValue() const;

        llvm::Value *GenOp(ASTOpExpr *Expr);

        llvm::Value *GenUnary(ASTUnaryOpExpr *Expr);

        llvm::Value *GenBinary(ASTBinaryOpExpr *Expr);

        llvm::Value *GenTernary(ASTTernaryOpExpr *Expr);

        llvm::Value *GenBinaryArith(const ASTExpr *E1, ASTBinaryOpExprKind OperatorKind, const ASTExpr *E2);

        llvm::Value *GenBinaryComparison(const ASTExpr *E1, ASTBinaryOpExprKind OperatorKind, const ASTExpr *E2);

        llvm::Value *GenBinaryLogic(const ASTExpr *E1, ASTBinaryOpExprKind OperatorKind, const ASTExpr *E2);
    };
}


#endif //FLY_CODEGEN_EXPR_H
