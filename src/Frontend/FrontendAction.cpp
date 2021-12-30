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
#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenGlobalVar.h"
#include "Parser/Parser.h"
#include "Basic/Debug.h"

using namespace fly;

FrontendAction::FrontendAction(const CompilerInstance & CI, ASTContext *Context, CodeGen &CG, InputFile *Input) :
        Context(Context), Diags(CI.getDiagnostics()), SourceMgr(CI.getSourceManager()), CG(CG), Input(Input) {
    FLY_DEBUG_MESSAGE("FrontendAction", "FrontendAction", "Input=" << Input->getFileName());
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
    FLY_DEBUG_MESSAGE("FrontendAction", "Parse", "Input=" << Input->getFileName());
    bool Success = true;
    Diags.getClient()->BeginSourceFile();

    if (P == nullptr) {
        // Create CodeGen
        CGM = CG.CreateModule(Input->getFileName());

        // Create AST
        AST = new ASTNode(Input->getFileName(), Context, CGM);

        // Create Parser and start to parse
        P = new Parser(*Input, SourceMgr, Diags);
        Success &= P->Parse(AST) && Context->AddNode(AST);
    }

    Diags.getClient()->EndSourceFile();
    return Success;
}

bool FrontendAction::HandleASTTopDecl() {
    assert(AST && "AST not built, need a Parse()");
    FLY_DEBUG_MESSAGE("FrontendAction", "HandleASTTopDecl", "Input=" << Input->getFileName());
    Diags.getClient()->BeginSourceFile();

    // Manage Imports
    for(const auto &I : AST->getImports()) {
        CGM->GenImport(I.getValue());
    }

    // Manage External GlobalVars
    for (const auto &Entry : AST->getExternalGlobalVars()) {
        FLY_DEBUG_MESSAGE("FrontendAction", "HandleASTTopDecl",
                          "ExternalGlobalVar=" << Entry.getValue()->str());
        CGM->GenGlobalVar(Entry.getValue(), true);
    }

    // Manage External Function
    for (const auto &EF : AST->getExternalFunctions()) {
        FLY_DEBUG_MESSAGE("FrontendAction", "HandleASTTopDecl",
                          "ExternalFunction=" << EF->str());
        CGM->GenFunction(EF, true);
    }

    // Manage GlobalVars
    std::vector<CodeGenGlobalVar *> CGGlobalVars;
    for (const auto &GV : AST->getGlobalVars()) {
        FLY_DEBUG_MESSAGE("FrontendAction", "HandleASTTopDecl",
                          "GlobalVar=" << GV.getValue()->str());
        CodeGenGlobalVar *CGV = CGM->GenGlobalVar(GV.getValue());
        CGGlobalVars.push_back(CGV);
    }

    // Instantiates all Function CodeGen in order to be set in all Call references
    std::vector<CodeGenFunction *> CGFunctions;
    for (ASTFunc *F : AST->getFunctions()) {
        FLY_DEBUG_MESSAGE("FrontendAction", "HandleASTTopDecl",
                          "Function=" << F->str());
        CodeGenFunction *CGF = CGM->GenFunction(F);
        CGFunctions.push_back(CGF);
    }

    // Body must be generated after all CodeGen has been set for each TopDecl
    for (auto &CGF : CGFunctions) {
        FLY_DEBUG_MESSAGE("FrontendAction", "HandleASTTopDecl",
                          "FunctionBody=" << CGF->getName());
        for (auto &CGV : CGGlobalVars) {
            CGV->reset();
        }
        CGF->GenBody();
    }

    Diags.getClient()->EndSourceFile();
    FLY_DEBUG_MESSAGE("FrontendAction", "HandleASTTopDecl",
                      "hasErrorOccurred=" << Diags.hasErrorOccurred());
    return !Diags.hasErrorOccurred();
}

bool FrontendAction::HandleTranslationUnit() {
    FLY_DEBUG_MESSAGE("FrontendAction", "Emit", "Input=" << Input->getFileName());
    Diags.getClient()->BeginSourceFile();
    OutputFile = CG.HandleTranslationUnit(CGM->Module);
    Diags.getClient()->EndSourceFile();
    return !Diags.hasErrorOccurred();
}

const std::string &FrontendAction::getOutputFile() const {
    return OutputFile;
}
