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

#include "CodeGenBase.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/IRBuilder.h>

namespace llvm {
	class Value;
}

namespace fly {

    class CodeGenModule;
    class SemaExpr;
	class SemaVar;
	class SemaCall;
	class SemaType;
	class SemaValue;
	class SemaBoolValue;
	class SemaIntValue;
	class SemaFloatValue;
	class SemaStringValue;
	class SemaArrayValue;
	class SemaStructValue;
	class SemaNullValue;
	class SemaEnumValue;
	class SemaMember;
	class SemaCast;
	class SemaUnary;
	class SemaBinary;
	class SemaTernary;
    enum class ASTBinaryKind;

    class CodeGenExpr : public CodeGenBase {

    protected:

        CodeGenModule * CGM;

    	llvm::Value *V;

    	llvm::IRBuilder<> * Builder;

	  public:

        CodeGenExpr(CodeGenModule *CGM);

    	virtual llvm::Value *getValue();

        void GenExpr(SemaVar *Sema);

        void GenExpr(SemaCall *Sema);

    	void GenExpr(SemaMember *Sema);

    	void GenExpr(SemaBoolValue *Sema);

    	void GenExpr(SemaIntValue *Sema);

    	void GenExpr(SemaFloatValue *Sema);

    	void GenExpr(SemaStringValue *Sema);

    	void GenExpr(SemaArrayValue *Sema);

    	void GenExpr(SemaStructValue *Sema);

    	void GenExpr(SemaNullValue *Sema);

    	void GenExpr(SemaEnumValue *Sema);

        void GenExpr(SemaCast *Sema);

        void GenExpr(SemaUnary *Sema);

        void GenExpr(SemaBinary *Sema);

        void GenExpr(SemaTernary *Sema);

        llvm::Value *GenBinaryArith(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2);

        llvm::Value *GenBinaryComparison(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2);

        llvm::Value *GenBinaryLogic(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2);

        llvm::Value* GenBinaryAssign(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2);

        void addArgs(SemaCall *Sema, llvm::SmallVector<llvm::Value *, 8> &Args);

    	llvm::Value *ConvertToBool(llvm::Value *V);

    	llvm::Value *Convert(llvm::Value *FromVal, SemaType *FromType, SemaType *ToType);
    };
}


#endif //FLY_CODEGEN_EXPR_H
