//===--------------------------------------------------------------------------------------------------------------===//
// include/Frontend/InputFile.h - Compiler Input File arguments
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_INPUTFILE_H
#define FLY_INPUTFILE_H

#include "Basic/SourceManager.h"
#include <llvm/Support/MemoryBuffer.h>

namespace fly {

    class InputOptions {

    };

    /// An input file for the front end.
    class InputFile {

        friend class FrontendAction;

        InputOptions Options;

        /// The file name, or "-" to read from standard input.
        llvm::StringRef File;

        FileID FID;

        /// The input, if it comes from a buffer rather than a file. This object
        /// does not own the buffer, and the caller is responsible for ensuring
        /// that it outlives any users.
        const llvm::MemoryBuffer *Buffer = nullptr;

    public:
        InputFile(llvm::StringRef File);

        bool Load(llvm::StringRef Source, SourceManager &SourceMgr);

        bool Load(SourceManager &SourceMgr, DiagnosticsEngine &Diags);

        bool isFile() const { return !File.empty(); }

        bool isBuffer() const { return Buffer != nullptr; }

        llvm::StringRef getFile() const {
            assert(isFile());
            return File;
        }

        FileID getFileID() const {
            assert(FID.isValid() && "Invalid FileID");
            return FID;
        }

        const llvm::MemoryBuffer *getBuffer() const {
            assert(isBuffer());
            return Buffer;
        }
    };
}

#endif //FLY_INPUTFILE_H
