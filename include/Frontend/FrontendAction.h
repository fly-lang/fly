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

#include "CompilerInstance.h"
#include "InputFile.h"
#include "FrontendOptions.h"
#include "OutputFile.h"
#include "ChainedDiagnosticConsumer.h"
#include "AST/ASTNode.h"
#include "Basic/FileManager.h"
#include "Parser/Parser.h"
#include "llvm/Support/VirtualFileSystem.h"

namespace fly {

    class FrontendAction {

        const InputFile &Input;

        ASTNode *AST;

        ASTContext *Context;

        FileManager &FileMgr;

        SourceManager &SourceMgr;

        DiagnosticsEngine &Diags;

        OutputFile Output;

        Parser *Parse;

    public:

        FrontendAction(const CompilerInstance &CI, InputFile &Input, ASTContext *Context);

        ~FrontendAction();

        bool BuildAST();

        ASTNode &getAST();
    };
}


#endif //FLY_FRONTENDACTION_H
