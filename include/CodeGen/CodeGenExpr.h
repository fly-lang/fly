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

        CodeGenExpr(CodeGenModule *CGM);

        llvm::Value *GenExpr(ASTExpr *Expr);

    private:

        llvm::Value* GenExpr(SemaExpr *Sema);

        CodeGenVarBase *GenExpr(SemaVar *Sema);

        llvm::Value *GenExpr(SemaCall *Sema);

        llvm::Value *GenExpr(SemaValue *Val);

        llvm::Value *GenCast(ASTExpr *Expr, SemaType *ToType);

        llvm::Value *GenUnary(ASTUnaryOp *Unary);

        llvm::Value *GenBinary(ASTBinaryOp *Binary);

        llvm::Value *GenTernary(ASTTernaryOp *Ternary);

        llvm::Value *GenBinaryArith(SemaExpr *E1, ASTBinaryOpKind OperatorKind, SemaExpr *E2);

        llvm::Value *GenBinaryComparison(SemaExpr *E1, ASTBinaryOpKind OperatorKind, SemaExpr *E2);

        llvm::Value *GenBinaryLogic(SemaExpr *E1, ASTBinaryOpKind OperatorKind, SemaExpr *E2);

        llvm::Value* GenBinaryAssign(SemaExpr *E1, ASTBinaryOpKind OperatorKind, SemaExpr *E2);

        void addArgs(SemaCall *Sema, llvm::SmallVector<llvm::Value*, 8>& Args);
    };
}


#endif //FLY_CODEGEN_EXPR_H
