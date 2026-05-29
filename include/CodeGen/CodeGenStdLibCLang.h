//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenStdLibCLang.h - fly.bridge.CLang code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_STDLIB_CLANG_H
#define FLY_CODEGEN_STDLIB_CLANG_H

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/IRBuilder.h>

#include <string>

namespace llvm {
class Value;
class Type;
}

namespace fly {

class CodeGenModule;
class SemaCall;
class SemaExpr;

class CodeGenStdLibCLang {

    CodeGenModule      *CGM;
    llvm::IRBuilder<>  *Builder;
    llvm::Value       *&V;

public:

    CodeGenStdLibCLang(CodeGenModule *CGM, llvm::IRBuilder<> *Builder, llvm::Value *&V);

    // Capture the lib literal argument after CLang constructor allocation.
    void GenConstructorCapture(SemaCall *Sema, llvm::Value *InstancePtr);

    // Emit code for CLang::call() — dynamic C function dispatch.
    void GenBridgeMethodCall(SemaCall *Sema);

private:

    void BuildCArgsFromArgsStruct(SemaExpr *ArgsExpr,
                                  llvm::SmallVector<llvm::Value *, 8> &CArgs,
                                  llvm::SmallVector<llvm::Type  *, 8> &CArgTys);

    void EmitCCallAndStoreResult(const std::string &SymStr,
                                 llvm::Type *CRetTy,
                                 bool IsVoid, bool IsPtr, bool IsBool,
                                 llvm::SmallVector<llvm::Value *, 8> &CArgs,
                                 llvm::SmallVector<llvm::Type  *, 8> &CArgTys,
                                 SemaExpr *OutExpr);

    static std::string NormalizeCLangLibFlag(const std::string &LibStr);
};

} // namespace fly

#endif // FLY_CODEGEN_STDLIB_CLANG_H
