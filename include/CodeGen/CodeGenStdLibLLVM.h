//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenStdLibLLVM.h - fly.llvm intrinsic code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_STDLIB_LLVM_H
#define FLY_CODEGEN_STDLIB_LLVM_H

#include <llvm/IR/IRBuilder.h>

namespace llvm { class Value; }

namespace fly {

class CodeGenModule;
class SemaCall;

class CodeGenStdLibLLVM {

    CodeGenModule      *CGM;
    llvm::IRBuilder<>  *Builder;
    llvm::Value       *&V;

public:

    CodeGenStdLibLLVM(CodeGenModule *CGM, llvm::IRBuilder<> *Builder, llvm::Value *&V);

    // Emit code for a fly.llvm intrinsic call.
    void GenCall(SemaCall *Sema);
};

} // namespace fly

#endif // FLY_CODEGEN_STDLIB_LLVM_H
