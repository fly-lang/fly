//===----------------------------------------------------------------------===//
// include/Compiler/CompilerInvocation.h - Chain Diagnostic Clients
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#ifndef FLY_COMPILERINVOCATION_H
#define FLY_COMPILERINVOCATION_H

#include "TextDiagnostic.h"
#include "TextDiagnosticPrinter.h"
#include "TextDiagnosticBuffer.h"
#include "InputFile.h"
#include "FrontendOptions.h"
#include "Basic/Diagnostic.h"
#include "Basic/SourceManager.h"
#include "Basic/FileManager.h"
#include "Basic/DiagnosticFrontend.h"
#include "Basic/TargetInfo.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ADT/StringRef.h>

namespace fly {

    using namespace llvm;

    /** Compiler Options **/
    class CompilerInvocation {

        /// The diagnostics engine instance.
        IntrusiveRefCntPtr<DiagnosticsEngine> diagnostics;

        /// The file manager.
        IntrusiveRefCntPtr<FileManager> fileMgr;

        /// The source manager.
        IntrusiveRefCntPtr<SourceManager> sourceMgr;

        /// The frontend options
        std::shared_ptr<FrontendOptions> frontendOptions;

        /// The target info
        IntrusiveRefCntPtr<TargetInfo> target;

        /// Create the file manager and replace any existing one with it.
        ///
        /// \return The new file manager on success, or null on failure.
        void createFileManager(IntrusiveRefCntPtr<llvm::vfs::FileSystem> vfs = nullptr);

        /// Create the source manager and replace any existing one with it.
        void createSourceManager();

        IntrusiveRefCntPtr<llvm::vfs::FileSystem> createVFSFromCompilerInvocation();

    public:

        CompilerInvocation(IntrusiveRefCntPtr<DiagnosticsEngine> diagnostics,
                           std::shared_ptr<FrontendOptions> frontendOptions,
                           IntrusiveRefCntPtr<TargetInfo> target);

        /// Get the current diagnostics engine.
        DiagnosticsEngine &getDiagnostics() const;

        /// Get the current file manager.
        FileManager &getFileManager() const;

        /// Get the current source mananger.
        SourceManager &getSourceManager() const;

        /// Get the current options.
        FrontendOptions &getFrontendOptions() const;

        /// Get the current target info.
        TargetInfo &getTargetInfo() const;
    };

}
#endif //FLY_COMPILERINVOCATION_H
