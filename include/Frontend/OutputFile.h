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

#include <string>

namespace fly {

/// Holds information about the output file.
///
/// If TempFilename is not empty we must rename it to Filename at the end.
/// TempFilename may be empty and Filename non-empty if creating the temporary
/// failed.
    class OutputFile {

        std::string file;
        std::string tempFile;

    public:

        void setFile(const std::string &file) {
            OutputFile::file = file;
            OutputFile::tempFile = file + ".tmp";
        }

        const std::string &getFile() const {
            return file;
        }

        const std::string &getTempFile() const {
            return tempFile;
        }
    };
}

#endif //FLY_OUTPUTFILE_H
