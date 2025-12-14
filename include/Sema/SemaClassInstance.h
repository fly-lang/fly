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

#include "llvm/ADT/DenseMap.h"

namespace fly {

    class SemaClassType;
    class CodeGenVar;

    class SemaClassInstance  : public SemaVar {

        friend class SemaBuilder;
        friend class SemaValidator;

        std::string Name = "this";

        SemaClassType *Class;

        uint64_t Index;

        llvm::DenseMap<size_t, SemaClassInstance *> BaseInstances;

		CodeGenVar *CodeGen = nullptr;

    protected:

        explicit SemaClassInstance(SemaClassType *Class);

    public:

        ~SemaClassInstance() override;

        SemaClassInstance *getParent() const override;

        llvm::StringRef getName() const override;

        const llvm::DenseMap<size_t, SemaClassInstance *> &getBaseInstances() const;

        SemaClassInstance *getBaseInstance(size_t TypeId) const;

        SemaClassType *getClass() const;

        uint64_t getIndex();

    	CodeGenVarBase *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase *CodeGen) override;

    };

}  // end namespace fly

#endif // FLY_SEMA_CLASS_INSTANCE_H