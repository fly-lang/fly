//===--------------------------------------------------------------------------------------------------------------===//
// src/Compiler/CompilerInstance.cpp - Instance of Compiler for a file
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/FrontendAction.h"
#include "Lex/Lexer.h"
#include "Parser/Parser.h"
#include "CodeGen/CodeGen.h"

using namespace fly;

FrontendAction::FrontendAction(const CompilerInstance & CI, const class InputFile & Input) :
        Input(Input), FrontendOpts(CI.getFrontendOptions()),
        CodeGenOpts(CI.getCodeGenOptions()), TargetOpts(CI.getTargetOptions()),
        Output(CI.getFrontendOptions().getOutputFile()),
        Diags(CI.getDiagnostics()), FileMgr(CI.getFileManager()),
        SourceMgr(CI.getSourceManager()), Target(CI.getTargetInfo()) {}

bool FrontendAction::Execute() {
    bool success = true;
    Diags.getClient()->BeginSourceFile();

    ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> result = FileMgr.getBufferForFile(Input.getFile());
    assert(!result.getError() && "Error while opening file");
    std::unique_ptr<MemoryBuffer> &buf = result.get();
    llvm::MemoryBuffer *b = buf.get();
    const FileID &FID = SourceMgr.createFileID(std::move(buf));

    if (!FrontendOpts.isSkipParse()) {
        Lexer lexer(FID, b, SourceMgr);

        Parser parser(Input.getFile(), lexer, Diags);
        if (Diags.hasErrorOccurred())
            return false;

        CodeGen CG(Diags, CodeGenOpts, TargetOpts, parser.getASTContext(), Target,
                   FrontendOpts.getBackendAction());
        success = CG.execute();
    }

    Diags.getClient()->EndSourceFile();
    return success;
}


