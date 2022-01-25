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

        bool Lib = false;
        std::string FileName;

    public:

        void setFile(std::string FileName, bool Lib = false) {
            OutputFile::FileName = FileName;
            OutputFile::Lib = Lib;
        }

        bool isLib() const {
            return Lib;
        }

        const std::string &getFile() const {
            return FileName;
        }
    };
}

#endif //FLY_OUTPUTFILE_H
