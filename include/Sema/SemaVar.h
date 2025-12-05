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

#include <llvm/ADT/StringRef.h>

#include "Sema/SemaExpr.h"

namespace fly {

    class ASTVar;
    class SemaType;
    class CodeGenVarBase;

	enum class SemaVarKind {
		PARAM_VAR,
		LOCAL_VAR,
		MEMBER_VAR,
		ERROR_VAR,
		CLASS_ATTRIBUTE,
		CLASS_INSTANCE,
		ENUM_ENTRY
	};

    class SemaVar : public SemaExpr {

        friend class SemaBuilder;

    	ASTVar *AST;

    	SemaVarKind VarKind;

    	bool Constant = false;

    protected:

        explicit SemaVar(ASTVar *AST, SemaVarKind Kind);

    public:
        virtual ~SemaVar() = default;

        ASTVar *getAST() const;

    	virtual llvm::StringRef getName() const;

    	SemaVarKind getVarKind() const;

    	bool isConstant() const;

    	virtual CodeGenVarBase *getCodeGen() const = 0;

        virtual void setCodeGen(CodeGenVarBase * CGVar) = 0;
    };

}

#endif //FLY_SEMA_VAR_H
