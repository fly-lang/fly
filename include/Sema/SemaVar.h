//===-------------------------------------------------------------------------------------------------------------===//
// include/Sym/SemaVar.h - Sybolic Table for ASTVar
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_VAR_H
#define FLY_SEMA_VAR_H

#include "Sema/SemaResult.h"
#include "AST/ASTVar.h"


namespace fly {

    class ASTVar;
    class SemaType;
    class CodeGenVarBase;

	enum class SemaVarKind {
		VAR_PARAM,
		VAR_LOCAL,
		VAR_MEMBER,
		VAR_ERROR,
		VAR_GLOBAL,
		VAR_CLASS,
		VAR_ENUM
	};

    class SemaVar : public SemaResult {

        friend class SemaBuilder;
        friend class SemaResolver;
    	friend class SemaResolverClass;
        friend class SemaValidator;

    	ASTVar *AST;

    	SemaVarKind VarKind;

    	bool Constant = false;

    protected:

        explicit SemaVar(ASTVar *AST, SemaVarKind Kind);

    public:
        virtual ~SemaVar() = default;

        ASTVar *getAST() const;

    	SemaVarKind getVarKind() const;

    	bool isConstant() const;

    	virtual CodeGenVarBase *getCodeGen() const = 0;

        virtual void setCodeGen(CodeGenVarBase * CGVar) = 0;
    };

}

#endif //FLY_SEMA_VAR_H
