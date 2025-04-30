//===-------------------------------------------------------------------------------------------------------------===//
// include/Sym/SymVar.h - Sybolic Table for ASTVar
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_VAR_H
#define FLY_SYM_VAR_H

#include <CodeGen/CodeGenError.h>


namespace fly {

    class ASTVar;
    class SymType;
    class CodeGenVarBase;

	enum class SymVarKind {
		VAR_PARAM,
		VAR_LOCAL,
		VAR_ERROR,
		VAR_GLOBAL,
		VAR_CLASS,
		VAR_ENUM
	};

    class SymVar {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

    	ASTVar *AST;

    	SymVarKind Kind;

    	SymVar *Parent = nullptr;

    	SymType *Type = nullptr;

    	bool Constant = false;

    protected:

        explicit SymVar(ASTVar *AST, SymVarKind Kind);

    public:
        virtual ~SymVar() = default;

        ASTVar *getAST() const;

    	SymVarKind getKind() const;

    	SymVar *getParent() const;

    	SymType *getType() const;

    	bool isConstant() const;

    	virtual CodeGenVarBase *getCodeGen() const = 0;

        virtual void setCodeGen(CodeGenVarBase * CGVar) = 0;
    };

}

#endif //FLY_SYM_VAR_H
