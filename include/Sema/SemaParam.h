//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaParam.h - Symbol Param
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_PARAM_H
#define FLY_SEMA_PARAM_H

#include "SemaVar.h"
#include "CodeGen/CodeGenVar.h"

namespace fly {

    class ASTParam;
	class CodeGenVar;

    class SemaParam : public SemaVar {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

		CodeGenVar *CodeGen;

        explicit SemaParam(ASTParam &AST, SemaType *Type);

    public:

        ~SemaParam() override = default;

        void accept(SemaVisitor& Visitor) override;
    };

}  // end namespace fly

#endif // FLY_SEMA_PARAM_H