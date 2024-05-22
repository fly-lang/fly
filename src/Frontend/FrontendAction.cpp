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
#include "AST/ASTFunction.h"
#include "AST/ASTIdentity.h"
#include "AST/ASTModule.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenGlobalVar.h"
#include "CodeGen/CodeGenClass.h"
#include "Parser/Parser.h"
#include "Basic/Debug.h"

using namespace fly;

FrontendAction::FrontendAction(const CompilerInstance & CI, CodeGen &CG, Sema &S, InputFile *Input) :
        Diags(CI.getDiagnostics()), SourceMgr(CI.getSourceManager()),
        FrontendOpts(CI.getFrontendOptions()), CG(CG), S(S), Input(Input) {
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
    CGM = CG.GenerateModule(Input->getFileName());
    if (FrontendOpts.CreateHeader) {
        CGH = CG.CreateHeader(Input->getFileName());
    }

    // Create Parser and start to parse
    P = new Parser(*Input, SourceMgr, Diags, S.getBuilder());
    P->Parse();

    // Set Module CodeGen
    Module->setCodeGen(CGM);

    if (Module && FrontendOpts.CreateHeader) {
        CGH->AddNameSpace(Module->getNameSpace());
    }

    return Module;
}

bool FrontendAction::ParseHeader() {
    assert(!P && "One time parse");
    FLY_DEBUG_MESSAGE("FrontendAction", "ParseHeader", "Input=" << Input->getFileName());

    // Create Parser and start to parse
    P = new Parser(*Input, SourceMgr, Diags, S.getBuilder());
    Module = P->ParseHeader();
    return Module && S.getBuilder().AddModule(Module);
}

bool FrontendAction::GenerateBodies() {
    assert(Module && "Module not built, need a Parse()");
    assert(!Module->isHeader() && "Cannot generate code from Header");
    FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode", "Input=" << Input->getFileName());
    Diags.getClient()->BeginSourceFile();

    // Body must be generated after all CodeGen has been set for each TopDecl
    for (auto CGF : CGFunctions) {
        FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                          "FunctionBody=" << CGF->getName());
        for (auto CGV : CGGlobalVars) { // All global vars need to be Loaded from Init()
            CGV->Init(); // FIXME only if used
        }
        CGF->GenBody();
    }

    // Generate Class Body
    if (CGClass) {
        for (auto CGCF: CGClass->getConstructors()) {
            CGCF->GenBody();
        }
        for (auto CGCF: CGClass->getFunctions()) {
            CGCF->GenBody();
        }
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
