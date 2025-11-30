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

    class ASTAttribute;
    class SemaComment;
    class SemaClassType;

    class SemaClassAttribute  : public SemaVar {

        friend class SemaBuilder;

        SemaClassType &Class;

        SemaVisibilityKind Visibility;

        bool Static = false;

        SemaClassType *Inherited = nullptr;

		CodeGenVar *CodeGen = nullptr;

        SemaComment *Comment = nullptr;

    protected:

        explicit SemaClassAttribute(ASTAttribute &AST, SemaClassType &Class);

    public:

        SemaClassType &getClass() const;

    	CodeGenVar *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase* CodeGen) override;

        SemaComment *getComment() const;

        SemaVisibilityKind getVisibility() const;

        bool isStatic() const;

        SemaClassType *getInherited() const;

    };

}  // end namespace fly

#endif // FLY_SEMA_CLASS_ATTRIBUTE_H