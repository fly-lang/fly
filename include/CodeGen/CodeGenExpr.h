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
#include "CodeGenModule.h"


namespace fly {

    class CodeGenModule;
    class ASTArithOpExpr;

    class CodeGenExpr {

        CodeGenModule * CGM;

        llvm::Value *Val;

        std::vector<llvm::Value *> PostValues;

        llvm::Function *Fn;

    public:
        CodeGenExpr(CodeGenModule *CGM, llvm::Function *Fn, ASTExpr *Expr, const ASTType *ToType);

        llvm::Value *getValue() const;

        llvm::Value *Convert(llvm::Value *V, const ASTType *FromType, const ASTType *ToType);

        llvm::Value *Convert(llvm::Value *V, llvm::Type *ToType, bool SignedInt = false);

        llvm::Value *GenValue(const ASTExpr *Origin, llvm::Value *Pointer = nullptr);

        llvm::Value *GenGroup(ASTGroupExpr *Group);

        bool hasOpPrecedence(BinaryOpKind Op);

        llvm::Value *GenUnary(ASTUnaryGroupExpr *Expr);

        llvm::Value *GenBinary(ASTBinaryGroupExpr *Expr);

        llvm::Value *GenTernary(ASTTernaryGroupExpr *Expr);

        Value *GenBinaryArith(const ASTExpr *E1, BinaryOpKind Op, const ASTExpr *E2);

        Value *GenBinaryComparison(const ASTExpr *E1, BinaryOpKind Op, const ASTExpr *E2);

        Value *GenBinaryLogic(const ASTExpr *E1, BinaryOpKind Op, const ASTExpr *E2);
    };
}


#endif //FLY_CODEGENEXPR_H
