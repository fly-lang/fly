//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassSymbols.h - SemaClassSymbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_CLASS_METHOD_H
#define FLY_SYM_CLASS_METHOD_H

#include "Sym/SymFunctionBase.h"
#include "CodeGen/CodeGenClassFunction.h"

namespace fly {

    class SymClass;
    class ASTFunction;
    class SymComment;
	class CodeGenClassFunction;

	enum class SymClassMethodKind {
		METHOD,
		METHOD_CONSTRUCTOR,
		METHOD_VIRTUAL
	};

    class SymClassMethod : public SymFunctionBase {

    	friend class SymBuilder;
    	friend class SemaResolver;
    	friend class SemaValidator;

    	enum class VisibilityKind {
    		PUBLIC,
			PROTECTED,
			PRIVATE,
		};

    	SymClass *Class;

    	SymClassMethodKind MethodKind;

    	bool Static = false;

    	SymClass *DerivedClass = nullptr;

		CodeGenClassFunction *CodeGen = nullptr;

    	SymComment *Comment = nullptr;

        explicit SymClassMethod(ASTFunction *AST);

    public:

    	SymClass *getClass() const;

    	bool SymClassMethod::isConstructor() const;

    	bool SymClassMethod::isStatic() const;

    	SymClass *getDerivedClass() const;

    	SymComment *getComment() const;

    	CodeGenClassFunction *getCodeGen() const override;

    	void setCodeGen(CodeGenClassFunction *CodeGen);
    };

}  // end namespace fly

#endif // FLY_SYM_CLASS_METHOD_H