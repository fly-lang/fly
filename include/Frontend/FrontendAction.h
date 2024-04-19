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

#include <string>
#include <vector>

namespace fly {

    class CodeGen;
    class CodeGenHeader;
    class SemaBuilder;
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
    class CodeGenGlobalVar;
    class CodeGenFunction;
    class CodeGenClass;
    class Sema;

    class FrontendAction {

        CodeGen &CG;

        Sema &S;

        Parser *P = nullptr;

        ASTNode *Node = nullptr;

        CodeGenModule *CGM = nullptr;

        CodeGenHeader *CGH = nullptr;

        InputFile *Input;

        SourceManager &SourceMgr;

        DiagnosticsEngine &Diags;

        FrontendOptions &FrontendOpts;

        std::string OutputFile;

        std::string HeaderFile;

        std::vector<CodeGenGlobalVar *> CGGlobalVars;

        std::vector<CodeGenFunction *> CGFunctions;

        CodeGenClass *CGClass  = nullptr;

        bool CGDone = false;

    public:

        FrontendAction(const CompilerInstance &CI, CodeGen &CG, Sema &S, InputFile *Input);

        ~FrontendAction();

        bool Parse();

        bool ParseHeader();

        void GenerateTopDef();

        bool GenerateBodies();

        bool HandleTranslationUnit();

        const std::string &getOutputFile() const;

        const std::string &getHeaderFile() const;
    };
}


#endif //FLY_FRONTENDACTION_H
