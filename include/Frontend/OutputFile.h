//===--------------------------------------------------------------------------------------------------------------===//
// include/Frontend/OutputFile.h - Compiler Output files
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_OUTPUTFILE_H
#define FLY_OUTPUTFILE_H

#include "llvm/ADT/StringRef.h"
#include <string>

namespace fly {

/// Holds information about the output file.
///
/// If TempFilename is not empty we must rename it to Filename at the end.
/// TempFilename may be empty and Filename non-empty if creating the temporary
/// failed.
    class OutputFile {

        llvm::StringRef file;
        llvm::StringRef tempFile;

    public:

        void setFile(const llvm::StringRef &file) {
            OutputFile::file = file;
            OutputFile::tempFile = llvm::StringRef(file.str() + ".tmp");
        }

        const llvm::StringRef &getFile() const {
            return file;
        }

        const llvm::StringRef &getTempFile() const {
            return tempFile;
        }
    };
}

#endif //FLY_OUTPUTFILE_H
