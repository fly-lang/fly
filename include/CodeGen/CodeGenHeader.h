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
    class ASTFunction;
    class ASTClass;
    class ASTType;
    class ASTModule;
    class CodeGenOptions;

    class CodeGenHeader {

    public:
        static void CreateFile(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts, ASTModule &Module);

        std::string SaveFile();

        void AddGlobalVar(ASTGlobalVar *GlobalVar);

        void AddFunction(ASTFunction *Func);

        void setClass(ASTClass *Class);

        const std::string Convert(ASTType *Type);

        void AddNameSpace(ASTNameSpace *pSpace);
    };
}

#endif //FLY_CODEGENHEADER_H
