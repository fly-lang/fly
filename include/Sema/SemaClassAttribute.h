//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassAttribute.h - class attribute semantic analysis
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
    class SemaExpr;
	enum class SemaVisibilityKind;

    class SemaClassAttribute  : public SemaVar {

        friend class SemaBuilder;
        friend class Resolver;

        SemaClassType &Class;

        SemaVisibilityKind Visibility;

        bool Static = false;

        SemaClassType *Inherited = nullptr;

		CodeGenVar *CodeGen = nullptr;

        SemaComment *Comment = nullptr;

        SemaExpr *InitExpr = nullptr;

    protected:

        explicit SemaClassAttribute(ASTAttribute &AST, SemaClassType &Class, SemaType *Type);

    public:

        ~SemaClassAttribute() override;

        SemaClassType &getClass() const;

        SemaComment *getComment() const;

        SemaVisibilityKind getVisibility() const;

        bool isStatic() const;

        SemaClassType *getInherited() const;

        SemaExpr *getInitExpr() const;

        void accept(SemaVisitor& Visitor) override;

    };

}  // end namespace fly

#endif // FLY_SEMA_CLASS_ATTRIBUTE_H