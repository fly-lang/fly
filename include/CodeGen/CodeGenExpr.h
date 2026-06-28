//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenExpr.h - expression code generation
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
	class SemaIntType;
	class SemaFloatType;
	class SemaNumberType;
	class SemaValue;
	class SemaBoolValue;
	class SemaIntValue;
	class SemaFloatValue;
	class SemaComplexValue;
	class SemaStringValue;
	class SemaArrayValue;
	class SemaStructValue;
	class SemaNullValue;
	class SemaUnsetValue;
	class SemaEnumEntry;
	class SemaEnumAccessor;
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

    	void GenExpr(SemaComplexValue *Sema);

    	void GenExpr(SemaStringValue *Sema);

    	void GenExpr(SemaStructValue *Sema);

    	void GenExpr(SemaNullValue *Sema);

    	void GenExpr(SemaUnsetValue *Sema);

    	void GenExpr(SemaEnumEntry *Sema);

    	void GenExpr(SemaEnumAccessor *Sema);

        void GenExpr(SemaCast *Sema);

        void GenExpr(SemaUnary *Sema);

        void GenExpr(SemaBinary *Sema);

        void GenExpr(SemaTernary *Sema);

    private:

        llvm::Value *GenBinaryArith(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2);

        llvm::Value *GenStringConcat(SemaExpr *E1, SemaExpr *E2);

        llvm::Value *GenStringHeapCopy(SemaStringValue *Sema);

        llvm::Value *GenStringClone(llvm::Value *StrVal);

        llvm::Value *GenBinaryCompare(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2);

        llvm::Value *GenBinaryLogic(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2);

        llvm::Value* GenBinaryAssign(SemaExpr *E1, SemaExpr *E2, bool FreeOldLHS = false);

        // Upcast adjustment: when a class pointer V (static type FromType) flows into a
        // base/interface-typed slot (ToType), return the pointer to the base subobject so
        // polymorphic dispatch reads the correct per-interface vtable. No-op otherwise.
        llvm::Value* adjustToBaseSubobject(llvm::Value *V, SemaType *FromType, SemaType *ToType);

        void addArgs(SemaCall *Sema, llvm::SmallVector<llvm::Value *, 8> &Args);

    	llvm::Value *ConvertToBool(llvm::Value *V);

    	llvm::Value *ConvertNumber(llvm::Value *V, SemaNumberType *Ty, bool IsSigned = true);

    	llvm::Value *ConvertToInteger(llvm::Value *V, SemaIntType *Ty);

    	llvm::Value *ConvertToFloat(llvm::Value *V, SemaFloatType *Ty, bool IsSigned = true);
    };
}


#endif //FLY_CODEGEN_EXPR_H
