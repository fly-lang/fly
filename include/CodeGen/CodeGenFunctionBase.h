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
}

namespace fly {

    class SymFunctionBase;
    class ASTVar;
    class CodeGenModule;

    class CodeGenFunctionBase {

    protected:

        SymFunctionBase *Sym = nullptr;

        CodeGenModule *CGM = nullptr;

        llvm::Function *Fn = nullptr;

        llvm::Type *RetType = nullptr;

        llvm::SmallVector<llvm::Type *, 8> ParamTypes;

        llvm::FunctionType *FnType = nullptr;

        llvm::BasicBlock *Entry = nullptr;

        llvm::Value *ErrorHandler = nullptr;

    public:
        CodeGenFunctionBase(CodeGenModule *CGM, SymFunctionBase *AST);

        CodeGenModule *getCodeGenModule();

        void GenReturnType();

        void GenParamTypes(CodeGenModule * CGM, llvm::SmallVector<llvm::Type *, 8> &Types, llvm::SmallVector<ASTVar *, 8> Params);

        SymFunctionBase *getSym();

        llvm::StringRef getName() const;

        llvm::Function *getFunction();

        llvm::FunctionType *getFunctionType();

        void setInsertPoint();

        void AllocaErrorHandler();

        void AllocaLocalVars();

        void StoreErrorHandler(bool isMain);

        void StoreParams(bool isMain);

        virtual void GenBody() = 0;
    };
}

#endif //FLY_CODEGEN_FUNCTIONBASE_H
