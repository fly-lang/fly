//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassSymbols.h - SemaClassSymbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_CLASS_ATTRIBUTE_H
#define FLY_SYM_CLASS_ATTRIBUTE_H

#include "Sym/SymVar.h"
#include "CodeGen/CodeGenClassVar.h"
#include "CodeGen/CodeGenVarBase.h"

namespace fly {

    class ASTVar;
    class SymComment;
    class SymClass;
	class CodeGenClassVar;

    class SymClassAttribute  : public SymVar {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        SymClass *Class;

        bool Static;

		CodeGenClassVar *CodeGen = nullptr;

        SymComment *Comment = nullptr;

    protected:

        explicit SymClassAttribute(ASTVar *AST);

    public:

        SymClass *getClass() const;

    	CodeGenClassVar *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase* CodeGen) override;

        SymComment *getComment() const;

        bool isStatic();
    };

}  // end namespace fly

#endif // FLY_SYM_CLASS_ATTRIBUTE_H