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

#include <AST/ASTOperatorExpr.h>
#include "AST/ASTExpr.h"
#include "CodeGenModule.h"


namespace fly {

    class CodeGenModule;
    class ASTOperatorExpr;
    class ASTArithExpr;

    class VirtualExpr : public ASTExpr {

        llvm::Value *Val;

    public:
        VirtualExpr(llvm::Value *Val) : Val(Val), ASTExpr(SourceLocation()) {}

        ASTExprKind getKind() const override {
            return ASTExprKind::EXPR_VIRTUAL;
        }

        ASTType *getType() const override {
            return nullptr;
        }

        llvm::Value *getVal() const {
            return Val;
        }

        std::string str() const override {
            return "{ Type=" + getType()->str() +
                   ", Kind=" + std::to_string(ASTExprKind::EXPR_VIRTUAL) +
                   ", Val=" + Val->getName().str() +
                   " }";
        }
    };

    class CodeGenExpr {

        CodeGenModule * CGM;

        llvm::Value *Val;

        std::vector<llvm::Value *> PostValues;

        llvm::Function *Fn;

    public:
        CodeGenExpr(CodeGenModule *CGM, llvm::Function *Fn, ASTExpr *Expr, const ASTType *Type);

        llvm::Value *getValue() const;

        llvm::Value *Generate(ASTExpr *Origin);

        llvm::Value *Convert(llvm::Value *V, const ASTType *ToType);

        llvm::Value *Convert(llvm::Value *V, llvm::Type *ToType);

        llvm::Value *GenValue(ASTExpr *Origin);

        llvm::Value *GenValue(ASTExpr *Origin, llvm::Value *&Pointer);

        llvm::Value *GenGroup(ASTGroupExpr *Origin, ASTGroupExpr *New, int Idx, ASTExpr *E1 = nullptr,
                              ASTOperatorExpr * OP1 = nullptr);

        bool hasOpPrecedence(ASTExpr *OP);

        bool canIterate(int Idx, ASTGroupExpr *Group);

        llvm::Value *OpUnary(ASTUnaryExpr *E);

        llvm::Value *OpBinary(ASTExpr *E1, ASTOperatorExpr *OP, ASTExpr *E2);

        Value *OpArith(ASTExpr *E1, ASTArithExpr *OP, ASTExpr *E2);

        Value *OpComparison(ASTExpr *E1, fly::ASTComparisonExpr *OP, ASTExpr *E2);

        Value *OpLogic(ASTExpr *E1, ASTLogicExpr *OP, ASTExpr *E2);
    };
}


#endif //FLY_CODEGENEXPR_H
