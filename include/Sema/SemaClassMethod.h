//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassSymbols.h - SemaClassSymbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_CLASS_METHOD_H
#define FLY_SEMA_CLASS_METHOD_H

#include "Sema/SemaFunctionBase.h"
#include "CodeGen/CodeGenClassFunction.h"

namespace fly {

    class SemaClassType;
    class ASTFunction;
    class SemaComment;
	class CodeGenClassFunction;

	enum class SemaClassMethodKind {
		METHOD,
		METHOD_CONSTRUCTOR,
		METHOD_VIRTUAL
	};

    class SemaClassMethod : public SemaFunctionBase {

    	friend class SemaBuilder;
    	friend class SemaResolver;
    	friend class SemaResolverClass;
    	friend class SemaValidator;

    	enum class VisibilityKind {
    		PUBLIC,
			PROTECTED,
			PRIVATE,
		};

    	SemaClassType *Class;

    	SemaClassMethodKind MethodKind;

    	bool Static = false;

    	SemaClassType *DerivedClass = nullptr;

		CodeGenClassFunction *CodeGen = nullptr;

    	SemaComment *Comment = nullptr;

        explicit SemaClassMethod(ASTFunction *AST, SemaClassType *Class);

    	std::string MangleFunction(ASTFunction *AST);

    public:

    	SemaClassType *getClass() const;

    	bool isConstructor() const;

    	bool isStatic() const;

    	SemaClassType *getDerivedClass() const;

    	SemaComment *getComment() const;

    	CodeGenClassFunction *getCodeGen() const override;

    	void setCodeGen(CodeGenClassFunction *CodeGen);
    };

}  // end namespace fly

#endif // FLY_SEMA_CLASS_METHOD_H