//===----------------------------------------------------------------------===//
// Compiler/CompilerInstance.cpp - Instance of Compiler for a file
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#include "Compiler/CompilerInstance.h"
#include "Lex/Lexer.h"
#include "Parser/Parser.h"
#include "CodeGen/CodeGen.h"

using namespace fly;

CompilerInstance::CompilerInstance(const CompilerInvocation & invocation, const InputFile & inputFile) :
        inputFile(inputFile), inputOptions(invocation.inputOptions),
        diagnostics(invocation.getDiagnostics()), fileMgr(invocation.getFileManager()),
        sourceMgr(invocation.getSourceManager()) {}

bool CompilerInstance::execute() {
    auto result = fileMgr.getBufferForFile(inputFile.getFile());
    assert(!result.getError() && "Error while opening file");
    unique_ptr<MemoryBuffer> &buf = result.get();
    llvm::MemoryBuffer *b = buf.get();
    const FileID &FID = sourceMgr.createFileID(std::move(buf));
    Lexer lexer(FID, b, sourceMgr);
    Parser parser(inputFile.getFile(), lexer);
    const CodeGen &codeGen = CodeGen(parser.getASTContext());
    return codeGen.execute();
}

CompilerInstance::~CompilerInstance() {

}

