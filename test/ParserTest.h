//
// Created by marco on 2/23/23.
//

#include "TestUtils.h"
#include "Parser/Parser.h"
#include "Sema/Sema.h"
#include "Sema/SemaBuilder.h"

#include <gtest/gtest.h>

#ifndef FLY_PARSERTEST_H
#define FLY_PARSERTEST_H

using namespace fly;

class ParserTest : public ::testing::Test {

public:
    const CompilerInstance CI;
    ASTContext *Context;
    SemaBuilder *Builder;
    DiagnosticsEngine &Diags;
    bool Success = false;

    ParserTest() : CI(*TestUtils::CreateCompilerInstance()),
                   Diags(CI.getDiagnostics()),
                   Builder(Sema::CreateBuilder(CI.getDiagnostics())) {
        Diags.getClient()->BeginSourceFile();
    }

    virtual ~ParserTest() {
        Diags.getClient()->EndSourceFile();
        Builder->Destroy();
    }

    ASTNode *Parse(std::string FileName, llvm::StringRef Source, bool DoBuild = true) {
        InputFile Input(Diags, CI.getSourceManager(), FileName);
        Input.Load(Source);
        Parser *P = new Parser(Input, CI.getSourceManager(), Diags, *Builder);
        ASTNode *Node = P->Parse();
        Success = !Diags.hasErrorOccurred() && Node;
        if (DoBuild)
            Success &= Builder->Build();
        return Node;
    }

    bool isSuccess() const {
        return Success;
    }

};

#endif //FLY_PARSERTEST_H
