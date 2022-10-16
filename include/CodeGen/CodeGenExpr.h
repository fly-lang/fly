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

#include "AST/ASTExpr.h"
#include "CodeGenModule.h"


namespace fly {

    class CodeGenModule;

    class CodeGenExpr {

        CodeGenModule * CGM = nullptr;

        llvm::Value *Val = nullptr;

        llvm::Function *Fn = nullptr;

    public:
        CodeGenExpr(CodeGenModule *CGM, llvm::Function *Fn, ASTExpr *Expr, const ASTType *ToType);

        llvm::Value *getValue() const;

        llvm::Value *Convert(llvm::Value *FromVal, const ASTType *FromType, const ASTType *ToType);

        llvm::Value *GenValue(const ASTExpr *Origin, llvm::Value *Pointer = nullptr);

        llvm::Value *GenGroup(ASTGroupExpr *Group);

        llvm::Value *GenUnary(ASTUnaryGroupExpr *Expr);

        llvm::Value *GenBinary(ASTBinaryGroupExpr *Expr);

        llvm::Value *GenTernary(ASTTernaryGroupExpr *Expr);

        Value *GenBinaryArith(const ASTExpr *E1, ASTBinaryOperatorKind Op, const ASTExpr *E2);

        bool isSigned(const ASTType * T1);

        Value *GenBinaryComparison(const ASTExpr *E1, ASTBinaryOperatorKind Op, const ASTExpr *E2);

        Value *GenBinaryLogic(const ASTExpr *E1, ASTBinaryOperatorKind Op, const ASTExpr *E2);
    };
}


#endif //FLY_CODEGEN_EXPR_H
