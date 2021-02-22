//===---------------------------------------------------------------------===//
// CodeGen/CodeGen.h - Gode Generation
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
/// \file
/// Defines the fly::CodeGen interface.
///
//===----------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_H
#define FLY_CODEGEN_H

#include "AST/ASTContext.h"

namespace fly {

    class CodeGen {

        const ASTContext &Context;

    public:
        explicit CodeGen(ASTContext &Context);

        bool execute() const;
    };
}

#endif //FLY_CODEGEN_H
