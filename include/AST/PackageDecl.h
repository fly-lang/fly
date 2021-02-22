//===----------------------------------------------------------------------===//
// AST/PackageDecl.h - Package Declaration
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
/// \file
/// Defines the fly::PackageDecl interface.
///
//===----------------------------------------------------------------------===//

#ifndef FLY_PACKAGEDECL_H
#define FLY_PACKAGEDECL_H

#include "ASTTypes.h"
#include <string>

namespace fly {

    using namespace std;

    class PackageDecl {

        const string &name;

        const ASTTypes type;

    public:

        PackageDecl(const PackageDecl&& packageDecl) noexcept;

        explicit PackageDecl(const string &name);

        const string &getName() const { return name; }

        const ASTTypes &getType() const { return type; }
    };
}

#endif //FLY_PACKAGEDECL_H
