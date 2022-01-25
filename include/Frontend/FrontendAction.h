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
    class FrontendOptions;

    class FrontendAction {

        CodeGen &CG;

        Parser *P = nullptr;

        ASTNode *AST = nullptr;

        CodeGenModule *CGM = nullptr;

        CodeGenHeader *CGH = nullptr;

        ASTContext *Context;

        InputFile *Input;

        SourceManager &SourceMgr;

        DiagnosticsEngine &Diags;

        FrontendOptions &FrontendOpts;

        std::string OutputFile;

        std::string HeaderFile;

        bool CGDone = false;

    public:

        FrontendAction(const CompilerInstance &CI, ASTContext *Context, CodeGen &CG, InputFile *Input);

        ~FrontendAction();

        ASTNode *getAST();

        bool Parse();

        bool ParseHeader();

        bool GenerateCode();

        bool HandleTranslationUnit();

        const std::string &getOutputFile() const;

        const std::string &getHeaderFile() const;
    };
}


#endif //FLY_FRONTENDACTION_H
