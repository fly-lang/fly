//===--------------------------------------------------------------------------------------------------------------===//
// include/Compiler/CompilerInvocation.h - Chain Diagnostic Clients
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_COMPILERINSTANCE_H
#define FLY_COMPILERINSTANCE_H

#include "TextDiagnostic.h"
#include "TextDiagnosticPrinter.h"
#include "TextDiagnosticBuffer.h"
#include "InputFile.h"
#include "FrontendOptions.h"
#include "Basic/Diagnostic.h"
#include "Basic/SourceManager.h"
#include "Basic/FileManager.h"
#include "Basic/TargetInfo.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ADT/StringRef.h>

namespace fly {

    using namespace llvm;

    /** Compiler Options **/
    class CompilerInstance {

        /// The diagnostics engine instance.
        IntrusiveRefCntPtr<DiagnosticsEngine> Diags;

        FileSystemOptions FileSystemOpts;

        /// The file manager.
        IntrusiveRefCntPtr<FileManager> FileMgr;

        /// The source manager.
        IntrusiveRefCntPtr<SourceManager> SourceMgr;

        /// The frontend options
        FrontendOptions *FrontendOpts;

        /// The frontend options
        CodeGenOptions *CodeGenOpts;

        /// The Target options
        std::shared_ptr<fly::TargetOptions> TargetOpts;

        /// Create the file manager and replace any existing one with it.
        ///
        /// \return The new file manager on success, or null on failure.
        void createFileManager(IntrusiveRefCntPtr<llvm::vfs::FileSystem> VFS = nullptr);

        /// Create the source manager and replace any existing one with it.
        void createSourceManager();

    public:

        CompilerInstance(IntrusiveRefCntPtr<DiagnosticsEngine> Diags,
                         FileSystemOptions &&FileSystemOpts,
                         std::shared_ptr<TargetOptions> &&TargetOpts,
                         FrontendOptions *FrontendOpts,
                         CodeGenOptions *CodeGenOpts);

        /// Get the current diagnostics engine.
        DiagnosticsEngine &getDiagnostics() const;

        /// Get the current file manager.
        FileManager &getFileManager() const;

        /// Get the current source mananger.
        SourceManager &getSourceManager() const;

        /// Get the current options.
        FrontendOptions &getFrontendOptions() const;

        CodeGenOptions &getCodeGenOptions() const;

        /// Get the current target options.
        const std::shared_ptr<fly::TargetOptions> &getTargetOptions() const;
    };

}
#endif //FLY_COMPILERINSTANCE_H
