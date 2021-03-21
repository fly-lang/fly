//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTContext.h - AST Context
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
/// \file
/// Defines the fly::ASTContext interface.
///
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTCONTEXT_H
#define FLY_ASTCONTEXT_H

#include "PackageDecl.h"
#include <string>
#include <llvm/ADT/StringRef.h>

using namespace llvm;

namespace fly {

    class ASTContext {

        const StringRef FileName;
        const PackageDecl Package;

    public:
        ASTContext(const StringRef &FileName, const PackageDecl &Package);

        const StringRef& getFileName() const;

        const PackageDecl& getPackage();

        void Release();
    };
}

#endif //FLY_ASTCONTEXT_H
