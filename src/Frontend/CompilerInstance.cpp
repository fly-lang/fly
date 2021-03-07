//===--------------------------------------------------------------------------------------------------------------===//
// src/Compiler/CompilerInstance.cpp - Instance of Compiler for a file
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/CompilerInstance.h"
#include "Lex/Lexer.h"
#include "Parser/Parser.h"
#include "CodeGen/CodeGen.h"

using namespace fly;

CompilerInstance::CompilerInstance(const CompilerInvocation & invocation, const InputFile & inputFile) :
        inputFile(inputFile), frontendOpts(invocation.getFrontendOptions()),
        outputFile(invocation.getFrontendOptions().getOutputFile()),
        diags(invocation.getDiagnostics()), fileMgr(invocation.getFileManager()),
        sourceMgr(invocation.getSourceManager()), target(invocation.getTargetInfo()) {}

bool CompilerInstance::execute() {
    diags.getClient()->BeginSourceFile();
    ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> result = fileMgr.getBufferForFile(inputFile.getFile());
    assert(!result.getError() && "Error while opening file");
    unique_ptr<MemoryBuffer> &buf = result.get();
    llvm::MemoryBuffer *b = buf.get();
    const FileID &FID = sourceMgr.createFileID(std::move(buf));
    bool success = true;
    if (!frontendOpts.isSkipParse()) {
        Lexer lexer(FID, b, sourceMgr);
        Parser parser(inputFile.getFile(), lexer, diags);
        const CodeGen &codeGen = CodeGen(diags, parser.getASTContext(), target);
        success = codeGen.execute();
    }
    diags.getClient()->EndSourceFile();
    return success;
}


