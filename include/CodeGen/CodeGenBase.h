//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenBase.h - code generator base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_BASE_H
#define FLY_CODEGEN_BASE_H

namespace fly {

    class CodeGenBase {

    protected:

    	explicit CodeGenBase() = default;

    public:
        virtual ~CodeGenBase() = default;

    };
}

#endif //FLY_CODEGEN_BASE_H
