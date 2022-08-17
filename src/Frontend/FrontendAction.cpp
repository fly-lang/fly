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
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenGlobalVar.h"
#include "Parser/Parser.h"
#include "Basic/Debug.h"

using namespace fly;

FrontendAction::FrontendAction(const CompilerInstance & CI, CodeGen &CG, SemaBuilder &Builder, InputFile *Input) :
        Diags(CI.getDiagnostics()), SourceMgr(CI.getSourceManager()),
        FrontendOpts(CI.getFrontendOptions()), CG(CG), Builder(Builder), Input(Input) {
    FLY_DEBUG_MESSAGE("FrontendAction", "FrontendAction", "Input=" << Input->getFileName());
    Diags.getClient()->BeginSourceFile();
}

FrontendAction::~FrontendAction() {
    FLY_DEBUG("FrontendAction", "~FrontendAction");
    Diags.getClient()->EndSourceFile();
    delete P;
    delete CGM;
    delete CGH;
}

bool FrontendAction::Parse() {
    assert(!P && "One time parse");
    FLY_DEBUG_MESSAGE("FrontendAction", "Parse", "Input=" << Input->getFileName());

    // Create CodeGen
    CGM = CG.CreateModule(Input->getFileName());
    if (FrontendOpts.CreateHeader) {
        CGH = CG.CreateHeader(Input->getFileName());
    }

    // Create Parser and start to parse
    P = new Parser(*Input, SourceMgr, Diags, Builder);
    Node = P->getNode();
    bool Success = P->Parse();
    if (FrontendOpts.CreateHeader) {
        CGH->AddNameSpace(Node->getNameSpace());
    }

    return Success;
}

bool FrontendAction::ParseHeader() {
    assert(!P && "One time parse");
    FLY_DEBUG_MESSAGE("FrontendAction", "ParseHeader", "Input=" << Input->getFileName());

    // Create Parser and start to parse
    P = new Parser(*Input, SourceMgr, Diags, Builder);
    Node = P->getNode();
    return P->ParseHeader() && Builder.AddNode(Node);
}

bool FrontendAction::GenerateCode() {
    assert(Node && "Node not built, need a Parse()");
    assert(!Node->isHeader() && "Cannot generate code from Header");
    FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode", "Input=" << Input->getFileName());
    Diags.getClient()->BeginSourceFile();

    // Manage External GlobalVars
    for (const auto &Entry : Node->getExternalGlobalVars()) {
        ASTGlobalVar *GlobalVar = Entry.getValue();
        FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                          "ExternalGlobalVar=" << GlobalVar->str());
        CGM->GenGlobalVar(GlobalVar, true);
    }

    // Manage External Function
    for (const auto &EF : Node->getExternalFunctions()) {
        FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                          "ExternalFunction=" << EF->str());
        CGM->GenFunction(EF, true);
    }

    // Manage GlobalVars
    std::vector<CodeGenGlobalVar *> CGGlobalVars;
    for (const auto &Entry : Node->getGlobalVars()) {
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
    for (auto &FuncStrMap : Node->getFunctions()) {
        for (auto &FuncList : FuncStrMap.getValue()) {
            for (auto &Func : FuncList.second) {

                FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                                  "Function=" << Func->str());
                CodeGenFunction *CGF = CGM->GenFunction(Func);
                CGFunctions.push_back(CGF);
                if (FrontendOpts.CreateHeader) {
                    CGH->AddFunction(Func);
                }
            }
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
