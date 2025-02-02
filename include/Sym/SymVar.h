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


namespace fly {

    class ASTVar;
    class CodeGenVarBase;

	enum class SymVarKind {
		VAR_PARAM,
		VAR_LOCAL,
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

        explicit SymVar(ASTVar *AST, SymVarKind Kind);

    public:

    	ASTVar *getAST() const;

    	SymVarKind getKind() const;

    	virtual CodeGenVarBase *getCodeGen() const = 0;

    };

}

#endif //FLY_SYM_VAR_H
