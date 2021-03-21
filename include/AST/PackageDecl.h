//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/PackageDecl.h - Package Declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
/// \file
/// Defines the fly::PackageDecl interface.
///
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_PACKAGEDECL_H
#define FLY_PACKAGEDECL_H

#include "ASTTypes.h"
#include <string>

namespace fly {

    class PackageDecl {

        const std::string &Name;

        const ASTTypes ASTType;

    public:

        PackageDecl(const PackageDecl&& Package) noexcept;

        explicit PackageDecl(const std::string &Name);

        const std::string &getName() const { return Name; }

        const ASTTypes &getType() const { return ASTType; }
    };
}

#endif //FLY_PACKAGEDECL_H
