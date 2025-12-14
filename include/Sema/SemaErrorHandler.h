//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaLocalVar.h - Sybolic Table for Local ASTVar
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

    class SemaErrorHandler : public SemaVar {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

    	SemaType *Type;

        CodeGenError *CodeGen;

    public:

        explicit SemaErrorHandler(ASTVar *AST);

        ~SemaErrorHandler() override = default;

        CodeGenError *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase *CodeGen) override;

    };

}

#endif //FLY_SEMA_ERRORHANDLER_H
