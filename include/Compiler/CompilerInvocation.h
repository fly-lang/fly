//===----------------------------------------------------------------------===//
// Compiler/CompilerInvocation.h - Chain Diagnostic Clients
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#ifndef FLY_COMPILERINVOCATION_H
#define FLY_COMPILERINVOCATION_H

#include "TextDiagnostic.h"
#include "TextDiagnosticPrinter.h"
#include "TextDiagnosticBuffer.h"
#include "InputFile.h"
#include "InputOptions.h"
#include "Basic/Diagnostic.h"
#include "Basic/SourceManager.h"
#include "Basic/FileManager.h"
#include "Basic/DiagnosticFrontend.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ADT/StringRef.h>

namespace fly {

    using namespace llvm;

    /** Compiler Options **/
    class CompilerInvocation {

        /// The diagnostics engine instance.
        DiagnosticsEngine &diagnostics;

        /// The file manager.
        IntrusiveRefCntPtr<FileManager> fileMgr;

        /// The source manager.
        IntrusiveRefCntPtr<SourceManager> sourceMgr;

        // Create Input Options and Files
        void createInputOptions(const std::string& path,
                                llvm::ArrayRef<const char *> argList);

        /// Create the file manager and replace any existing one with it.
        ///
        /// \return The new file manager on success, or null on failure.
        void createFileManager(IntrusiveRefCntPtr<llvm::vfs::FileSystem> vfs = nullptr);

        /// Create the source manager and replace any existing one with it.
        void createSourceManager();

        IntrusiveRefCntPtr<llvm::vfs::FileSystem> createVFSFromCompilerInvocation(DiagnosticsEngine &diagnostics);

    public:

        /// The input options.
        const InputOptions inputOptions;

        /// The input files.
        SmallVector<InputFile, 0> inputFiles;

        explicit CompilerInvocation(DiagnosticsEngine &diagnostics,
                                    const std::string& path,
                                    ArrayRef<const char *> argList);

        /// Get the current diagnostics engine.
        DiagnosticsEngine &getDiagnostics() const {
            return diagnostics;
        }

        /// Get the current diagnostics engine.
        FileManager &getFileManager() const {
            assert(fileMgr && "Compiler instance has no file manager!");
            return *fileMgr;
        }

        /// Get the current diagnostics engine.
        SourceManager &getSourceManager() const {
            assert(sourceMgr && "Compiler instance has no file manager!");
            return *sourceMgr;
        }
    };

}
#endif //FLY_COMPILERINVOCATION_H
