//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenHelper.h - CodeGen helper utilities
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_HELPER_H
#define FLY_CODEGEN_HELPER_H

#include <string>

namespace fly {

    class ASTFunction;
    class ASTNameSpace;
    class ASTType;
    class SemaFunctionBase;
    class SemaType;

    class CodeGenHelper {
    public:
        static std::string Mangle(SemaType *T);
        static std::string Mangle(SemaFunctionBase *F);
        static std::string Mangle(ASTType *T);
        static std::string Mangle(const std::string &NS, ASTFunction *F);
        static std::string Mangle(const std::string &NS, const std::string &ClassName, ASTFunction *F);
        static std::string Mangle(ASTNameSpace *NS, ASTFunction *F);
        static std::string Mangle(ASTNameSpace *NS, const std::string &ClassName, ASTFunction *F);
        static std::string FlattenNS(ASTNameSpace *NS);
    };
}

#endif //FLY_CODEGEN_HELPER_H
