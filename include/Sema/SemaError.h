//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaError.h - semantic error types
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_ERRORHANDLER_H
#define FLY_SEMA_ERRORHANDLER_H

#include "SemaVar.h"
#include "CodeGen/CodeGenError.h"

namespace fly {

    class ASTVar;
    class SemaType;
    class CodeGenVar;

    class SemaError : public SemaVar {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

    	SemaType *Type = nullptr;

    public:

        explicit SemaError(ASTVar *AST);

        ~SemaError() override = default;

        CodeGenError *getCodeGen() const override;

        void setCodeGen(CodeGenError *CodeGen);

        void accept(SemaVisitor& Visitor) override;

    };

}

#endif //FLY_SEMA_ERRORHANDLER_H
