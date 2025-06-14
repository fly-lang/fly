//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassSymbols.h - SemaClassSymbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_CLASS_INSTANCE_H
#define FLY_SEMA_CLASS_INSTANCE_H

#include "Sema/SemaVar.h"

namespace fly {

    class SemaClassType;
	class CodeGenVar;

    class SemaClassInstance  : public SemaVar {

        friend class SemaBuilder;
        friend class SemaResolverClass;
        friend class SemaValidator;

        SemaClassType *Class;

		CodeGenVar *CodeGen = nullptr;

    protected:

        explicit SemaClassInstance(SemaClassType *Class);

    public:

        SemaClassType *getClass() const;

    	CodeGenVarBase *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase* CodeGen) override;

    };

}  // end namespace fly

#endif // FLY_SEMA_CLASS_INSTANCE_H