//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaVar.h - variable semantic analysis
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
#include "Sema/SemaAlloc.h"

#include <llvm/ADT/StringRef.h>

namespace fly {

    class ASTVar;
    class SemaType;
    class CodeGenVar;
    class SemaSmartAlloc;
    class SemaStringAlloc;

    class SemaVar : public SemaExpr {

        friend class SemaBuilder;

    protected:

    	ASTVar *AST;

    	bool Constant = false;

    	// Non-owning: owned by the enclosing SemaBlockStmt (via addAlloc). Do NOT delete here.
    	// Holds either a SemaSmartAlloc (smart pointer) or SemaStringAlloc (heap string).
    	SemaAlloc *Alloc = nullptr;

        explicit SemaVar(ASTVar *AST, SemaKind Kind, SemaType *Type);

    public:
        virtual ~SemaVar();

        std::string str() const override;

        ASTVar *getAST() const;

    	virtual llvm::StringRef getName() const;

    	bool isConstant() const;

    	SemaAlloc *getAlloc() const;
    	void setAlloc(SemaAlloc *A);

    	SemaSmartAlloc *getSmartAlloc() const;
    	SemaStringAlloc *getStringAlloc() const;

    	CodeGenVar *getCodeGen() const override;

        void setCodeGen(CodeGenVar * CodeGen);
    };

}

#endif //FLY_SEMA_VAR_H
