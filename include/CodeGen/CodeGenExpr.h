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
#include <llvm/ADT/SmallVector.h>

namespace llvm {
	class Value;
}

namespace fly {

    class CodeGenModule;
    class ASTUnary;
    class ASTBinary;
    class ASTTernary;
    class ASTOp;
    class ASTIdentifier;
    class SemaExpr;
    class ASTExpr;
	class SemaVar;
	class SemaCall;
	class SemaType;
	class SemaValue;
	class SemaCast;
	class SemaUnary;
	class SemaBinary;
	class SemaTernary;
    enum class ASTBinaryKind;

    class CodeGenExpr {

        CodeGenModule * CGM;

    public:

        CodeGenExpr(CodeGenModule *CGM);

    	llvm::Value *GenExpr(SemaExpr *Sema);

    private:

        llvm::Value *GenExpr(SemaVar *Sema);

        llvm::Value *GenExpr(SemaCall *Sema);

        llvm::Value *GenExpr(SemaValue *Sema);

        llvm::Value *GenExpr(SemaCast *Sema);

        llvm::Value *GenExpr(SemaUnary *Sema);

        llvm::Value *GenExpr(SemaBinary *Sema);

        llvm::Value *GenExpr(SemaTernary *Sema);

        llvm::Value *GenBinaryArith(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2);

        llvm::Value *GenBinaryComparison(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2);

        llvm::Value *GenBinaryLogic(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2);

        llvm::Value* GenBinaryAssign(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2);

        void addArgs(SemaCall *Sema, llvm::SmallVector<llvm::Value *, 8> &Args);
    };
}


#endif //FLY_CODEGEN_EXPR_H
