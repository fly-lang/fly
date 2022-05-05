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
#include "Sema/SemaBuilder.h"
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

FrontendAction::FrontendAction(const CompilerInstance & CI, ASTContext *Context, CodeGen &CG, SemaBuilder &Builder,
                               InputFile *Input) :
        Context(Context), Diags(CI.getDiagnostics()), SourceMgr(CI.getSourceManager()),
        FrontendOpts(CI.getFrontendOptions()), CG(CG), Builder(Builder), Input(Input) {
    FLY_DEBUG_MESSAGE("FrontendAction", "FrontendAction", "Input=" << Input->getFileName());
    Diags.getClient()->BeginSourceFile();
}

FrontendAction::~FrontendAction() {
    FLY_DEBUG("FrontendAction", "~FrontendAction");
    Diags.getClient()->EndSourceFile();
    delete P;
    delete AST;
    delete CGM;
    delete CGH;
}

ASTNode *FrontendAction::getAST() {
    assert(AST && "AST not built, need a Parse()");
    return AST;
}

bool FrontendAction::Parse() {
    FLY_DEBUG_MESSAGE("FrontendAction", "Parse", "Input=" << Input->getFileName());
    bool Success = true;

    if (P == nullptr) {
        // Create CodeGen
        CGM = CG.CreateModule(Input->getFileName());
        if (FrontendOpts.CreateHeader) {
            CGH = CG.CreateHeader(Input->getFileName());
        }

        // Create AST
        AST = new ASTNode(Input->getFileName(), Context, CGM);

        // Create Parser and start to parse
        P = new Parser(*Input, SourceMgr, Diags, Builder);
        Success = P->Parse(AST);
        if (FrontendOpts.CreateHeader) {
            CGH->AddNameSpace(AST->getNameSpace());
        }
    }

    return Success;
}

bool FrontendAction::ParseHeader() {
    FLY_DEBUG_MESSAGE("FrontendAction", "ParseHeader", "Input=" << Input->getFileName());
    bool Success = true;
    Diags.getClient()->BeginSourceFile();

    if (P == nullptr) {
        // Create AST
        ASTNode *Node = new ASTNode(Input->getFileName(), Context);

        // Create Parser and start to parse
        P = new Parser(*Input, SourceMgr, Diags, Builder);
        Success &= P->ParseHeader(Node) && Context->AddNode(Node);
    }

    Diags.getClient()->EndSourceFile();
    return Success;
}

bool FrontendAction::GenerateCode() {
    assert(AST && "AST not built, need a Parse()");
    assert(!AST->isHeader() && "Cannot generate code from Header");
    FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode", "Input=" << Input->getFileName());
    Diags.getClient()->BeginSourceFile();

    // Manage External GlobalVars
    for (const auto &Entry : AST->getExternalGlobalVars()) {
        ASTGlobalVar *GlobalVar = Entry.getValue();
        FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                          "ExternalGlobalVar=" << GlobalVar->str());
        CGM->GenGlobalVar(GlobalVar, true);
    }

    // Manage External Function
    for (const auto &EF : AST->getExternalFunctions()) {
        FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                          "ExternalFunction=" << EF->str());
        CGM->GenFunction(EF, true);
    }

    // Manage GlobalVars
    std::vector<CodeGenGlobalVar *> CGGlobalVars;
    for (const auto &Entry : AST->getGlobalVars()) {
        ASTGlobalVar *GlobalVar = Entry.getValue();
        FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                          "GlobalVar=" << GlobalVar->str());
        CodeGenGlobalVar *CGV = CGM->GenGlobalVar(GlobalVar);
        CGGlobalVars.push_back(CGV);
        if (FrontendOpts.CreateHeader) {
            CGH->AddGlobalVar(GlobalVar);
        }
    }

    // Instantiates all Function CodeGen in order to be set in all Call references
    std::vector<CodeGenFunction *> CGFunctions;
    for (ASTFunction *Func : AST->getFunctions()) {
        FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                          "Function=" << Func->str());
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGFunctions.push_back(CGF);
        if (FrontendOpts.CreateHeader) {
            CGH->AddFunction(Func);
        }
    }

    // Body must be generated after all CodeGen has been set for each TopDecl
    for (auto &CGF : CGFunctions) {
        FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                          "FunctionBody=" << CGF->getName());
        for (auto &CGV : CGGlobalVars) {
            CGV->Init();
        }
        CGF->GenBody();
    }

    Diags.getClient()->EndSourceFile();
    FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                      "hasErrorOccurred=" << Diags.hasErrorOccurred());
    CGDone = !Diags.hasErrorOccurred();
    return CGDone;
}

bool FrontendAction::HandleTranslationUnit() {
    assert(CGDone && "Code not generated successfully");
    FLY_DEBUG_MESSAGE("FrontendAction", "HandleTranslationUnit", "Input=" << Input->getFileName());
    Diags.getClient()->BeginSourceFile();
    OutputFile = CG.HandleTranslationUnit(CGM->Module);
    if (FrontendOpts.CreateHeader) {
        HeaderFile = CGH->GenerateFile();
    }
    Diags.getClient()->EndSourceFile();
    return !Diags.hasErrorOccurred();
}

const std::string &FrontendAction::getOutputFile() const {
    return OutputFile;
}

const std::string &FrontendAction::getHeaderFile() const {
    return HeaderFile;
}
