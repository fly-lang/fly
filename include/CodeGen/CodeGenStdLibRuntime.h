//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenStdLibRuntime.h - fly.runtime C symbol code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_STDLIB_RUNTIME_H
#define FLY_CODEGEN_STDLIB_RUNTIME_H

#include <llvm/IR/IRBuilder.h>

namespace llvm { class Value; }

namespace fly {

class CodeGenModule;
class SemaCall;

class CodeGenStdLibRuntime {

    CodeGenModule      *CGM;
    llvm::IRBuilder<>  *Builder;
    llvm::Value       *&V;

public:

    CodeGenStdLibRuntime(CodeGenModule *CGM, llvm::IRBuilder<> *Builder, llvm::Value *&V);

    // Emit code for a fly.runtime C symbol call.
    void GenCall(SemaCall *Sema);
};

} // namespace fly

#endif // FLY_CODEGEN_STDLIB_RUNTIME_H
