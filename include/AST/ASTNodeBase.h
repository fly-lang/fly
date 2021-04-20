//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNodeBase.h - Base AST Node
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTNODEBASE_H
#define FLY_ASTNODEBASE_H

#include "Basic/SourceLocation.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/DenseMap.h"

namespace fly {

    class ASTContext;
    class ASTNode;

    class ASTNodeBase {

    protected:

        ASTContext* Context;

        // Unit FileName
        const llvm::StringRef FileName;

        // File ID
        const FileID FID;

    public:

        ASTNodeBase() = delete;

        ASTNodeBase(const StringRef &FileName, const FileID &FID, ASTContext* Context);

        const FileID &getFileID() const;

        const llvm::StringRef& getFileName();

        ASTContext &getContext() const;
    };
}

#endif //FLY_ASTNODEBASE_H
