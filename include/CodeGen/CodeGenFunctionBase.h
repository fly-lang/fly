//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGFunction.h - Code Generator of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_FUNCTIONBASE_H
#define FLY_CODEGEN_FUNCTIONBASE_H

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/SmallVector.h"

namespace llvm {
    class Function;
    class FunctionType;
    class Type;
    class BasicBlock;
    class StructType;
    class Value;
    class AllocaInst;
}

namespace fly {

	class SemaType;
    class SemaFunctionBase;
    class ASTVar;
    class CodeGenModule;

    class CodeGenFunctionBase {

    protected:

        SemaFunctionBase *Sema;

        CodeGenModule *CGM;

        llvm::Function *Fn = nullptr;

        llvm::Type *RetType = nullptr;

        llvm::SmallVector<llvm::Type *, 8> ParamTypes;

        llvm::FunctionType *FnType = nullptr;

        llvm::BasicBlock *Entry = nullptr;

    public:
        CodeGenFunctionBase(CodeGenModule *CGM, SemaFunctionBase *Sema);

        CodeGenModule *getCodeGenModule();

        void GenReturnType();

        static void GenParamTypes(CodeGenModule *CGM, llvm::SmallVector<llvm::Type *, 8> &Types, SemaFunctionBase *Sema);

        SemaFunctionBase *getSema();

        llvm::StringRef getName() const;

        llvm::Function *getFunction();

        llvm::FunctionType *getFunctionType();

        void setInsertPoint();

        void GenDebugSubprogram();

        void AllocaLocalVars();

        void StoreParams(size_t Idx);

        void CheckReturnVoid();

        virtual void GenBody() = 0;
    };
}

#endif //FLY_CODEGEN_FUNCTIONBASE_H
