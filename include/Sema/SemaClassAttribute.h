//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassSymbols.h - SemaClassSymbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_CLASS_ATTRIBUTE_H
#define FLY_SEMA_CLASS_ATTRIBUTE_H

#include "Sema/SemaVar.h"
#include "CodeGen/CodeGenClassVar.h"
#include "CodeGen/CodeGenVarBase.h"

namespace fly {

    class ASTVar;
    class SemaComment;
    class SemaClassType;
	class CodeGenClassVar;

    class SemaClassAttribute  : public SemaVar {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        SemaClassType *Class;

        bool Static;

		CodeGenClassVar *CodeGen = nullptr;

        SemaComment *Comment = nullptr;

    protected:

        explicit SemaClassAttribute(ASTVar *AST, SemaClassType *Class);

    public:

        SemaClassType *getClass() const;

    	CodeGenClassVar *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase* CodeGen) override;

        SemaComment *getComment() const;

        bool isStatic();
    };

}  // end namespace fly

#endif // FLY_SEMA_CLASS_ATTRIBUTE_H