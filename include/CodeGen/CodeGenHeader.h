//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenHeader.h - Header Generator
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

namespace fly {
    class DiagnosticsEngine;
    class SymNameSpace;
    class ASTFunction;
    class ASTClass;
    class ASTModule;
    class CodeGenOptions;
    class ASTVar;

    class CodeGenHeader
    {
    public:
        static void CreateFile(DiagnosticsEngine& Diags, CodeGenOptions& CodeGenOpts, SymNameSpace& NameSpace);

        std::string SaveFile();

        void AddGlobalVar(ASTVar* GlobalVar);

        void AddFunction(ASTFunction* Func);

        void setClass(ASTClass* Class);

        const std::string Convert(ASTTypeRef* TypeRef);

        void AddNameSpace(SymNameSpace* pSpace);
    };
}

#endif //FLY_CODEGENHEADER_H
