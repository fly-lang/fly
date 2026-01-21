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

#include "CodeGen/CodeGenVar.h"
#include "Sema/SemaExpr.h"

#include <llvm/ADT/StringRef.h>

namespace fly {

    class ASTVar;
    class SemaType;
    class CodeGenVar;

    class SemaVar : public SemaExpr {

        friend class SemaBuilder;

    protected:

    	ASTVar *AST;

    	bool Constant = false;

    	CodeGenVar *CodeGen;

        explicit SemaVar(ASTVar *AST, SemaKind Kind, SemaType *Type);

    public:
        virtual ~SemaVar() = default;

        ASTVar *getAST() const;

    	virtual llvm::StringRef getName() const;

    	bool isConstant() const;

    	CodeGenVar *getCodeGen() const override;

        void setCodeGen(CodeGenVar * CodeGen);
    };

}

#endif //FLY_SEMA_VAR_H
