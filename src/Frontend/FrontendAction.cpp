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

using namespace fly;

FrontendAction::FrontendAction(const CompilerInstance & CI, const class InputFile & Input, ASTContext *Context) :
        Input(Input), Output(CI.getFrontendOptions().getOutputFile()), Context(Context),
        Diags(CI.getDiagnostics()), FileMgr(CI.getFileManager()),
        SourceMgr(CI.getSourceManager()) {

    ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> result = FileMgr.getBufferForFile(Input.getFile());
    assert(!result.getError() && "Error while opening file");
    std::unique_ptr<MemoryBuffer> &Buf = result.get();
    MemoryBuffer *BufPtr = Buf.get();
    FID = SourceMgr.createFileID(std::move(Buf));

    // Create Lexer
    Lexer Lex(FID, BufPtr, SourceMgr);

    // Create Parser and start to parse
    P = std::make_unique<Parser>(Lex, Diags);
}

bool FrontendAction::BuildAST() {

    // Create AST Unit
    AST = std::make_unique<ASTNode>(Input.getFile(), FID, Context);

    return P->Parse(AST.get());
}

ASTNode& FrontendAction::getAST() {
    return *AST;
}
