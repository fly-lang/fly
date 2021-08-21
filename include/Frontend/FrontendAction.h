//===--------------------------------------------------------------------------------------------------------------===//
// include/Compiler/CompilerInstance.h - Chain Diagnostic Clients
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_FRONTENDACTION_H
#define FLY_FRONTENDACTION_H

#include <CodeGen/CodeGen.h>
#include "OutputFile.h"

namespace fly {

    class CodeGen;
    class CodeGenModule;
    class CompilerInstance;
    class ASTNode;
    class ASTContext;
    class Parser;
    class DiagnosticsEngine;
    class InputFile;
    class FileManager;
    class SourceManager;

    class FrontendAction {

        CodeGen &CG;

        Parser *P = nullptr;

        ASTNode *AST = nullptr;

        CodeGenModule *CGM = nullptr;

        ASTContext *Context;

        SourceManager &SourceMgr;

        DiagnosticsEngine &Diags;

    public:

        FrontendAction(const CompilerInstance &CI, ASTContext *Context, CodeGen &CG);

        ~FrontendAction();

        ASTNode *getAST();

        bool Parse(InputFile &Input);

        bool Compile();

        bool EmitOutput();
    };
}


#endif //FLY_FRONTENDACTION_H
