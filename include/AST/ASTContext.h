//===----------------------------------------------------------------------===//
// AST/ASTContext.h - AST Context
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
/// \file
/// Defines the fly::ASTContext interface.
///
//===----------------------------------------------------------------------===//

#ifndef FLY_ASTCONTEXT_H
#define FLY_ASTCONTEXT_H

#include "PackageDecl.h"
#include <string>

namespace fly {

    using namespace std;

    class ASTContext {

        const string fileName;
        const PackageDecl package;

    public:
        ASTContext(const string &fileName, const PackageDecl &package);

        const string& getFileName();

        const PackageDecl& getPackage();

        void Release();
    };
}

#endif //FLY_ASTCONTEXT_H
