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

#include <CodeGen/CodeGenVar.h>

#include "Sema/SemaVar.h"

namespace fly {

    class ASTVar;
    class SemaComment;
    class SemaClassType;
	class CodeGenVar;

    class SemaClassAttribute  : public SemaVar {

        friend class SemaBuilder;
        friend class SemaResolverClass;
        friend class SemaValidator;

        SemaClassType *Class;

        SemaVisibilityKind Visibility = SemaVisibilityKind::DEFAULT;

        bool Static = false;

        uint64_t Index;

		CodeGenVar *CodeGen = nullptr;

        SemaComment *Comment = nullptr;

    protected:

        explicit SemaClassAttribute(ASTVar *AST, SemaClassType *Class, uint64_t Index);

    public:

        SemaClassType *getClass() const;

        uint64_t getIndex() const;

    	CodeGenVar *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase* CodeGen) override;

        SemaComment *getComment() const;

        SemaVisibilityKind getVisibility() const;

        bool isStatic();
    };

}  // end namespace fly

#endif // FLY_SEMA_CLASS_ATTRIBUTE_H