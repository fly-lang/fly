//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/BackendUtil.h - Code Generator Utils
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_BACKENDUTIL_H
#define FLY_BACKENDUTIL_H

#include "Basic/CodeGenOptions.h"
#include "Basic/Diagnostic.h"
#include "Basic/TargetOptions.h"
#include "Basic/LLVM.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include <memory>

namespace fly {

    enum BackendActionKind {
        Backend_EmitAssembly,  ///< Emit native assembly files
        Backend_EmitBC,        ///< Emit LLVM bitcode files
        Backend_EmitLL,        ///< Emit human-readable LLVM assembly
        Backend_EmitNothing,   ///< Don't emit anything (benchmarking mode)
        Backend_EmitObj        ///< Emit native object files
    };

    void EmitBackendOutput(DiagnosticsEngine &Diags, const CodeGenOptions &CGOpts, const TargetOptions &TOpts,
                           bool TimePasses, const llvm::DataLayout &TDesc, llvm::Module *M, BackendActionKind Action,
                           std::unique_ptr<raw_pwrite_stream> OS);

    void EmbedBitcode(llvm::Module *M, const CodeGenOptions &CGOpts, llvm::MemoryBufferRef Buf);

    llvm::Expected<llvm::BitcodeModule> FindThinLTOModule(llvm::MemoryBufferRef MBRef);

    llvm::BitcodeModule * FindThinLTOModule(llvm::MutableArrayRef<llvm::BitcodeModule> BMs);
}

#endif //FLY_BACKENDUTIL_H
