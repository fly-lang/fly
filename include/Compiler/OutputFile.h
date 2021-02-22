//===----------------------------------------------------------------------===//
// Compiler/OutputFile.h - Compiler Output files
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

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

        std::string Filename;
        std::string TempFilename;

    public:
        OutputFile() = default;
//        CompilerOutputFile(std::string filename, std::string tempFilename)
//        : Filename(std::move(filename)), TempFilename(std::move(tempFilename)) {
//        }

        const std::string &getFilename() const {
            return Filename;
        }

        const std::string &getTempFilename() const {
            return TempFilename;
        }
    };
}

#endif //FLY_OUTPUTFILE_H
