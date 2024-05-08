//
// Created by marco on 2/23/23.
//

#include "TestUtils.h"
#include "Parser/Parser.h"
#include "Sema/Sema.h"
#include "Sema/SemaBuilder.h"
#include "Sys/Sys.h"

#include <gtest/gtest.h>

#ifndef FLY_PARSERTEST_H
#define FLY_PARSERTEST_H

using namespace fly;

class ParserTest : public ::testing::Test {

public:
    const CompilerInstance CI;
    ASTContext *Context;
    Sema *S;
    DiagnosticsEngine &Diags;
    bool Success = false;

    ParserTest() : CI(*TestUtils::CreateCompilerInstance()),
                   Diags(CI.getDiagnostics()) {
        S = Sema::CreateSema(CI.getDiagnostics());
        Diags.getClient()->BeginSourceFile();
    }

    virtual ~ParserTest() {
        Diags.getClient()->EndSourceFile();
        delete S;
    }

    ASTModule *Parse(std::string FileName, llvm::StringRef Source, bool DoBuild = true) {
        InputFile Input(Diags, CI.getSourceManager(), FileName);
        Input.Load(Source);
        Parser *P = new Parser(Input, CI.getSourceManager(), Diags, *S->getBuilder());
        ASTModule *Module = P->Parse();
        Success = !Diags.hasErrorOccurred() && Module;
        if (DoBuild)
            Success &= S->Resolve();
        return Module;
    }

    bool isSuccess() const {
        return Success;
    }

};

#endif //FLY_PARSERTEST_H
