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

    enum FileExt {
        UNKNOWN,
        FLY,
        LIB,
        OBJ,
        HEAD
    };

    /// An input file for the front end.
    class InputFile {

        friend class FrontendAction;

        DiagnosticsEngine &Diags;

        SourceManager &SourceMgr;

        /// The file name, or "-" to read from standard input.
        const std::string FileName;

        const std::string Name;

        const  FileExt Ext;

        FileID FID;

        /// The input, if it comes from a buffer rather than a file. This object
        /// does not own the buffer, and the caller is responsible for ensuring
        /// that it outlives any users.
        const llvm::MemoryBuffer *Buffer = nullptr;

    public:
        explicit InputFile(DiagnosticsEngine &Diags, SourceManager &SourceMgr, std::string FileName);

        bool Load(llvm::StringRef Source);

        bool Load();

        bool isEmpty() const;

        bool isBuffer() const;

        const std::string &getFileName() const;

        const std::string &getName() const;

        const FileExt &getExt() const;

        FileID getFileID() const;

        const llvm::MemoryBuffer *getBuffer() const;

    };
}

#endif //FLY_INPUTFILE_H
