//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaParam.h - function parameter semantic analysis
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

        // True only for the hidden output params the Resolver appends for the
        // return convention ('out' / __out_N). Matching them by NAME broke every
        // function whose USER-declared trailing param happens to be called `out`
        // (the runtime convention: strSize, mem_alloc, …).
        bool Synthetic = false;

        explicit SemaParam(ASTParam &AST, SemaType *Type);

    public:

        ~SemaParam() override = default;

        bool isSynthetic() const;

        std::string str() const override;

        void accept(SemaVisitor& Visitor) override;
    };

}  // end namespace fly

#endif // FLY_SEMA_PARAM_H