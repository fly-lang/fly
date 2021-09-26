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

namespace fly {

    class ASTContext;

    class ASTNodeBase {

    protected:

        ASTContext* Context;

        // Node FileName
        const llvm::StringRef FileName;

    public:

        ASTNodeBase() = delete;

        ASTNodeBase(const llvm::StringRef &FileName, ASTContext* Context);

        const llvm::StringRef& getFileName();

        ASTContext &getContext() const;

        virtual std::string str() const;
    };
}

#endif //FLY_ASTNODEBASE_H
