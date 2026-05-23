//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenType.h - type code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_TYPE_H
#define FLY_CODEGEN_TYPE_H

#include "CodeGenBase.h"

namespace llvm {
	class Type;
	class PointerType;
}

namespace fly {

	class CodeGenModule;
	class SemaBoolType;
	class SemaIntType;
	class SemaFloatType;
	class SemaComplexType;
	class SemaArrayType;
	class SemaErrorType;
	class SemaVoidType;
	class SemaStringType;
	class SemaEnumType;
	class SemaClassType;

    class CodeGenType : public CodeGenBase {

    protected:

    	llvm::Type *T = nullptr;

        CodeGenModule *CGM;

    public:

        CodeGenType(CodeGenModule *CGM);

    	llvm::Type *getType();
    	
    	void GenType(SemaBoolType &Sema);

    	void GenType(SemaIntType &Sema);

    	void GenType(SemaFloatType &Sema);

    	void GenType(SemaComplexType &Sema);

    	void GenType(SemaArrayType &Sema);

    	void GenType(SemaErrorType &Sema);

    	void GenType(SemaVoidType &Sema);

    	void GenType(SemaStringType &Sema);

    	void GenType(SemaEnumType &Sema);
    };
}


#endif //FLY_CODEGEN_EXPR_H
