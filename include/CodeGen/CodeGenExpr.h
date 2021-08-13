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

        ExprKind getKind() const override {
            return ExprKind::EXPR_VIRTUAL;
        }

        ASTType *getType() const {
            return nullptr;
        }

        llvm::Value *getVal() const {
            return Val;
        }
    };

    class CodeGenExpr {

        CodeGenModule * CGM;

        llvm::Value *Val;

        std::vector<llvm::Value *> PostValues;

    public:
        CodeGenExpr(CodeGenModule *CGM, ASTExpr *Expr, const ASTType *Type);

        llvm::Value *getValue() const;

        llvm::Value *Generate(ASTExpr *Origin);

        llvm::Value *Convert(llvm::Value *V, const ASTType *ToType);

        llvm::Value *Convert(llvm::Value *V, llvm::Type *ToType);

        llvm::Value *GenValue(ASTExpr *Origin);

        llvm::Value *GenGroup(ASTGroupExpr *Origin, ASTGroupExpr *New, int Idx, ASTExpr *E1 = nullptr,
                              ASTOperatorExpr * OP1 = nullptr);

        bool hasOpPrecedence(ASTExpr *OP);

        bool canIterate(int Idx, ASTGroupExpr *Group);

        llvm::Value *OpUnary(ASTUnaryExpr *E);

        llvm::Value *OpBinary(ASTExpr *E1, ASTOperatorExpr *OP, ASTExpr *E2);

        Value *OpArith(llvm::Value *V1, ASTArithExpr *OP, llvm::Value *V2);

        Value *OpComparison(llvm::Value *V1, fly::ASTComparisonExpr *OP, llvm::Value *V2);

        Value *OpLogic(llvm::Value *V1, ASTLogicExpr *OP, llvm::Value *V2);
    };
}


#endif //FLY_CODEGENEXPR_H
