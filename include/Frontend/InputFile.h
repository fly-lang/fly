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

#include <llvm/Support/MemoryBuffer.h>

namespace fly {

    class InputOptions {

    };

/// An input file for the front end.
    class InputFile {

        InputOptions Options;

        /// The file name, or "-" to read from standard input.
        std::string File;

        /// The input, if it comes from a buffer rather than a file. This object
        /// does not own the buffer, and the caller is responsible for ensuring
        /// that it outlives any users.
        const llvm::MemoryBuffer *Buffer = nullptr;

    public:
        InputFile(llvm::StringRef File)
                : File(File.str()) {}

        InputFile(const llvm::MemoryBuffer *Buffer)
                : Buffer(Buffer) {}

        bool isEmpty() const { return File.empty() && Buffer == nullptr; }

        bool isFile() const { return !isBuffer(); }

        bool isBuffer() const { return Buffer != nullptr; }

        std::string getFile() const {
            assert(isFile());
            return File;
        }

        const llvm::MemoryBuffer *getBuffer() const {
            assert(isBuffer());
            return Buffer;
        }
    };
}

#endif //FLY_INPUTFILE_H
