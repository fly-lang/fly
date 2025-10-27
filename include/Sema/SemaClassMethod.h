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
#include "CodeGen/CodeGenClassMethod.h"
#include "Sema/SemaVisibilityKind.h"

namespace fly {

    class SemaClassType;
    class ASTFunction;
    class SemaComment;
	class SemaClassInstance;
	class CodeGenClassMethod;

	enum class SemaClassMethodKind {
		METHOD,
		METHOD_CONSTRUCTOR,
		METHOD_ABSTRACT
	};

    class SemaClassMethod : public SemaFunctionBase {

    	friend class SemaBuilder;
    	friend class Resolver;
    	friend class SemaResolverClass;
    	friend class SemaValidator;

    protected:

    	SemaClassType *Class;

    	SemaClassInstance *This;

    	SemaClassMethodKind MethodKind;

    	SemaVisibilityKind Visibility = SemaVisibilityKind::DEFAULT;

    	bool Static = false;

    	SemaClassMethod *Overridden = nullptr;

		CodeGenClassMethod *CodeGen = nullptr;

    	SemaComment *Comment = nullptr;

    	explicit SemaClassMethod(ASTFunction *AST, SemaClassType *Class,  SemaClassInstance *This, SemaClassMethodKind MethodKind);

    	std::string MangleFunction(ASTFunction *AST);

    public:

    	SemaClassType *getClass() const;
    	
    	SemaClassInstance *getThis() const;

    	bool isConstructor() const;

    	bool isAbstract() const;

    	SemaVisibilityKind getVisibility() const;

    	bool isStatic() const;

    	SemaClassMethod *getOverridden() const;

    	SemaComment *getComment() const;

    	CodeGenClassMethod *getCodeGen() const override;

    	void setCodeGen(CodeGenClassMethod *CodeGen);
    };

}  // end namespace fly

#endif // FLY_SEMA_CLASS_METHOD_H