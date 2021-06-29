//===--------------------------------------------------------------------------------------------------------------===//
// src/Compiler/CompilerInstance.cpp - Instance of Compiler for a file
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/FrontendAction.h"

using namespace fly;

FrontendAction::FrontendAction(const CompilerInstance & CI, InputFile & Input, ASTContext *Context) :
        Input(Input), Output(CI.getFrontendOptions().getOutputFile()), Context(Context),
        Diags(CI.getDiagnostics()), FileMgr(CI.getFileManager()),
        SourceMgr(CI.getSourceManager()) {

    Input.Load(SourceMgr, Diags);

    // Create Parser and start to parse
    Parse = new Parser(Input, SourceMgr, Diags);
}

bool FrontendAction::BuildASTNode() {
    // Create AST Unit
    AST = new ASTNode(Input.getFile(), Input.getFileID(), Context);
    return Parse->Parse(AST);
}

ASTNode& FrontendAction::getASTNode() {
    return *AST;
}

FrontendAction::~FrontendAction() {
    delete Parse;
    delete AST;
}
