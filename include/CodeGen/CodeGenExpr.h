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
    class ASTUnaryOp;
    class ASTBinaryOp;
    class ASTTernaryOp;
    class ASTOp;
    class ASTIdentifier;
    class SemaExpr;
    class ASTExpr;
    enum class ASTBinaryOpKind;

    class CodeGenExpr {

        CodeGenModule * CGM;

    public:

        static llvm::Value* Generate(CodeGenModule* CGM, ASTExpr* Expr);

    private:

        CodeGenExpr(CodeGenModule *CGM);

        llvm::Value *GenExpr(ASTExpr *Expr);

        llvm::Value *GenValue(SemaType *Type, SemaValue *Val);
        
        llvm::Value *GenOp(ASTOp *Expr);

        llvm::Value *GenUnary(ASTUnaryOp *Unary);

        llvm::Value *GenBinary(ASTBinaryOp *Binary);

        llvm::Value *GenTernary(ASTTernaryOp *Ternary);

        llvm::Value *GenBinaryArith(ASTExpr *E1, ASTBinaryOpKind OperatorKind, ASTExpr *E2);

        llvm::Value *GenBinaryComparison(ASTExpr *E1, ASTBinaryOpKind OperatorKind, ASTExpr *E2);

        llvm::Value *GenBinaryLogic(ASTExpr *E1, ASTBinaryOpKind OperatorKind, ASTExpr *E2);
    };
}


#endif //FLY_CODEGEN_EXPR_H
