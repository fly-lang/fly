//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTInstance.h - Instance for Call or VarRef
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_INSTANCE_H
#define FLY_AST_INSTANCE_H

namespace fly {

    class CodeGenInstance;

    class ASTInstance {

    public:

        virtual ASTInstance *getInstance() const = 0;

        virtual bool isCall() const = 0;
    };
}

#endif //FLY_AST_INSTANCE_H
