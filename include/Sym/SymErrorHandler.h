//===-------------------------------------------------------------------------------------------------------------===//
// include/Sym/SymLocalVar.h - Sybolic Table for Local ASTVar
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_ERRORHANDLER_H
#define FLY_SYM_ERRORHANDLER_H

#include "SymVar.h"
#include "CodeGen/CodeGenVar.h"

namespace fly {

    class ASTVar;
    class SymType;
    class CodeGenVar;

    class SymErrorHandler : public SymVar {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

    	SymType *Type;

        CodeGenError *CodeGen;

    public:

        explicit SymErrorHandler();

        CodeGenError *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase *CodeGen) override;

    };

}

#endif //FLY_SYM_ERRORHANDLER_H
