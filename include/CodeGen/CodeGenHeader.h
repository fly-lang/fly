//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenHeader.h - header file code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_HEADER_H
#define FLY_CODEGEN_HEADER_H

#include <vector>
#include <string>
#include <AST/ASTType.h>

namespace fly {
    class DiagnosticsEngine;
    class SemaNameSpace;
    class ASTFunction;
    class ASTClass;
    class ASTModule;
    class CodeGenOptions;
    class ASTVar;

    class CodeGenHeader
    {
    public:
        static void CreateFile(DiagnosticsEngine& Diags, CodeGenOptions& CodeGenOpts, SemaNameSpace& NameSpace);

        std::string SaveFile();

        void AddGlobalVar(ASTVar* GlobalVar);

        void AddFunction(ASTFunction* Func);

        void setClass(ASTClass* Class);

        const std::string Convert(ASTType* TypeRef);

        void AddNameSpace(SemaNameSpace* pSpace);
    };
}

#endif //FLY_CODEGENHEADER_H
