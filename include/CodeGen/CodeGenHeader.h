//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenHeader.h - Header Generator
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGENHEADER_H
#define FLY_CODEGENHEADER_H

#include <vector>
#include <string>

namespace fly {

    class DiagnosticsEngine;
    class ASTNameSpace;
    class ASTGlobalVar;
    class ASTFunc;
    class ASTType;
    class CodeGenOptions;

    class CodeGenHeader {

        DiagnosticsEngine &Diags;

        CodeGenOptions &CodeGenOpts;

        std::string Name;

        ASTNameSpace *NameSpace;

        std::vector<ASTGlobalVar *> GlobalVars;

        std::vector<ASTFunc *> Functions;

    public:
        CodeGenHeader(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts, std::string Name);

        std::string GenerateFile();

        void AddGlobalVar(ASTGlobalVar *GlobalVar);

        void AddFunction(ASTFunc *Func);

        const std::string &Convert(ASTType *Type);

        void AddNameSpace(ASTNameSpace *pSpace);
    };
}

#endif //FLY_CODEGENHEADER_H
