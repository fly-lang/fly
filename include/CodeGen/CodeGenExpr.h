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
    class ASTUnaryGroupExpr;
    class ASTBinaryGroupExpr;
    class ASTTernaryGroupExpr;
    enum class ASTBinaryOperatorKind;

    class CodeGenExpr {

        CodeGenModule * CGM = nullptr;

        llvm::Value *Val = nullptr;

        ASTVar *Var = nullptr;

    public:

        CodeGenExpr(CodeGenModule *CGM, ASTExpr *Expr);

        llvm::Value *GenValue(const ASTExpr *Expr);

        llvm::Value *getValue() const;

        llvm::Value *Convert(llvm::Value *FromVal, const ASTType *FromType, const ASTType *ToType);

        llvm::Value *GenGroup(ASTGroupExpr *Group);

        llvm::Value *GenUnary(ASTUnaryGroupExpr *Expr);

        llvm::Value *GenBinary(ASTBinaryGroupExpr *Expr);

        llvm::Value *GenTernary(ASTTernaryGroupExpr *Expr);

        llvm::Value *GenBinaryArith(const ASTExpr *E1, ASTBinaryOperatorKind OperatorKind, const ASTExpr *E2);

        llvm::Value *GenBinaryComparison(const ASTExpr *E1, ASTBinaryOperatorKind OperatorKind, const ASTExpr *E2);

        llvm::Value *GenBinaryLogic(const ASTExpr *E1, ASTBinaryOperatorKind OperatorKind, const ASTExpr *E2);
    };
}


#endif //FLY_CODEGEN_EXPR_H
