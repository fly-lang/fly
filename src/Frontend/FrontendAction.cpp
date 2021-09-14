//===--------------------------------------------------------------------------------------------------------------===//
// src/Compiler/CompilerInstance.cpp - Instance of Compiler for a file
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/FrontendAction.h"
#include "Frontend/CompilerInstance.h"
#include "Frontend/InputFile.h"
#include "AST/ASTContext.h"
#include "AST/ASTNode.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "Parser/Parser.h"

using namespace fly;

FrontendAction::FrontendAction(const CompilerInstance & CI, ASTContext *Context, CodeGen &CG) :
        Context(Context), Diags(CI.getDiagnostics()), SourceMgr(CI.getSourceManager()), CG(CG) {
}

FrontendAction::~FrontendAction() {
    delete P;
    delete AST;
}

ASTNode *FrontendAction::getAST() {
    assert(AST && "AST not built, need a Parse()");
    return AST;
}

bool FrontendAction::Parse(InputFile & Input) {
    bool Success = false;
    Diags.getClient()->BeginSourceFile();

    if (P == nullptr) {
        // Create CodeGen
        CGM = CG.CreateModule(Input.getFile());

        // Create AST
        AST = new ASTNode(Input.getFile(), Context, CGM);

        // Create Parser and start to parse
        P = new Parser(Input, SourceMgr, Diags);
        Success &= P->Parse(AST) && Context->AddNode(AST);
    }

    Diags.getClient()->EndSourceFile();
    return Success;
}

bool FrontendAction::Compile() {
    assert(AST && "AST not built, need a Parse()");
    Diags.getClient()->BeginSourceFile();

    // Manage Top Decl
    AST->getGlobalVars().begin();
    for (const auto &V : AST->getGlobalVars()) {
        CGM->GenGlobalVar(V.getValue());
    }
    for (ASTFunc *F : AST->getFunctions()) {
        CGM->GenFunction(F);
    }

    Diags.getClient()->EndSourceFile();
    return Diags.hasErrorOccurred();
}

bool FrontendAction::EmitOutput() {
    Diags.getClient()->BeginSourceFile();
    CG.Emit(CGM);
    Diags.getClient()->EndSourceFile();
    return Diags.hasErrorOccurred();
}