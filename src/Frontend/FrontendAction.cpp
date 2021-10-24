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
#include "AST/ASTImport.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "Parser/Parser.h"
#include "Basic/Debug.h"

using namespace fly;

FrontendAction::FrontendAction(const CompilerInstance & CI, ASTContext *Context, CodeGen &CG, InputFile &Input) :
        Context(Context), Diags(CI.getDiagnostics()), SourceMgr(CI.getSourceManager()), CG(CG), Input(Input) {
    FLY_DEBUG_MESSAGE("FrontendAction", "FrontendAction", "Input=" << Input.getFile());
}

FrontendAction::~FrontendAction() {
    FLY_DEBUG("FrontendAction", "~FrontendAction");
    delete P;
    delete AST;
}

ASTNode *FrontendAction::getAST() {
    assert(AST && "AST not built, need a Parse()");
    return AST;
}

CodeGenModule *FrontendAction::getCodeGenModule() const {
    assert(AST && "CGM not built, need a Parse()");
    return CGM;
}

bool FrontendAction::Parse() {
    FLY_DEBUG_MESSAGE("FrontendAction", "Parse", "Input=" << Input.getFile());
    bool Success = true;
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

bool FrontendAction::HandleASTTopDecl() {
    assert(AST && "AST not built, need a Parse()");
    FLY_DEBUG_MESSAGE("FrontendAction", "HandleASTTopDecl", "Input=" << Input.getFile());
    Diags.getClient()->BeginSourceFile();

    // Manage Imports
    for(const auto &I : AST->getImports()) {
        CGM->GenImport(I.getValue());
    }

    // Manage External GlobalVars
    for (const auto &EGV : AST->getExternalGlobalVars()) {
        FLY_DEBUG_MESSAGE("FrontendAction", "HandleASTTopDecl",
                          "ExternalGlobalVar=" << EGV->str());
        CGM->GenGlobalVar(EGV, true);
    }

    // Manage External Function
    for (const auto &EF : AST->getExternalFunctions()) {
        FLY_DEBUG_MESSAGE("FrontendAction", "HandleASTTopDecl",
                          "ExternalFunction=" << EF->str());
        CGM->GenFunction(EF, true);
    }

    // Manage GlobalVars
    for (const auto &GV : AST->getGlobalVars()) {
        FLY_DEBUG_MESSAGE("FrontendAction", "HandleASTTopDecl",
                          "GlobalVar=" << GV.getValue()->str());
        CGM->GenGlobalVar(GV.getValue());
    }

    // Manage Functions
    for (ASTFunc *F : AST->getFunctions()) {
        CGM->GenFunction(F);
    }

    Diags.getClient()->EndSourceFile();
    return !Diags.hasErrorOccurred();
}

bool FrontendAction::HandleTranslationUnit() {
    FLY_DEBUG_MESSAGE("FrontendAction", "Emit", "Input=" << Input.getFile());
    Diags.getClient()->BeginSourceFile();
    CG.HandleTranslationUnit(CGM->Module);
    Diags.getClient()->EndSourceFile();
    return !Diags.hasErrorOccurred();
}
