//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenHeader.cpp - Header Generator implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenHeader.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTType.h"
#include "Basic/Diagnostic.h"
#include "Basic/CodeGenOptions.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunc.h"
#include "Basic/Debug.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"

#include <string>


using namespace fly;

CodeGenHeader::CodeGenHeader(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts, std::string Name) :
    Diags(Diags), CodeGenOpts(CodeGenOpts), Name(Name) {

}

std::string CodeGenHeader::GenerateFile() {
    std::string Header;
    FLY_DEBUG_MESSAGE("CodeGenHeader", "GenerateFile","FileName=" << Name);
    std::string FileHeader = Name + ".h";
    llvm::StringRef FileName = llvm::sys::path::filename(FileHeader);
    FLY_DEBUG_MESSAGE("CodeGenHeader", "GenerateFile","FileName=" << FileName);

    // generate namespace
    Header = "namespace " + NameSpace->getName() + "\n";

    // generate global var declarations
    for (auto &GlobalVar : GlobalVars) {
        if (GlobalVar->getVisibility() == V_PUBLIC) {
            Header += "\npublic " + Convert(GlobalVar->getType()) + "." +
                      GlobalVar->getName() + "\n";
        }
    }

    // generate function declarations
    for (auto &Function : Functions) {
        if (Function->getVisibility() == V_PUBLIC) {
            Header += "\npublic " + Convert(Function->getType()) + " " + Function->getName() +
                      "(";
            int i = 0;
            for (auto &Param : Function->getHeader()->getParams()) {
                Header += Convert(Param->getType()) + " " + Param->getName();
                if (i < Function->getHeader()->getParams().size()) {
                    Header += ",";
                }
            }
            Header += ")\n";
        }
    }

    int FD;
    const std::error_code &EC = llvm::sys::fs::openFileForWrite(FileName, FD,
                                                                llvm::sys::fs::CD_CreateAlways,
                                                                llvm::sys::fs::F_None);
    if (EC) {
        Diags.Report(diag::err_generate_header) << EC.message();
        return "";
    }

    llvm::raw_fd_ostream file(FD, false);

    // Write the data.
    StringRef StrRef(Header);
    file.write(StrRef.data(), StrRef.size());

    // write to file
    return FileName.str();
}

void CodeGenHeader::AddNameSpace(ASTNameSpace *NS) {
    this->NameSpace = NS;
}

void CodeGenHeader::AddGlobalVar(ASTGlobalVar *GlobalVar) {
    GlobalVars.push_back(GlobalVar);
}

void CodeGenHeader::AddFunction(ASTFunc *Func) {
    Functions.push_back(Func);
}

const std::string CodeGenHeader::Convert(ASTType *Type) {
    switch (Type->getKind()) {
        case TYPE_INT:
            return "int";
        case TYPE_FLOAT:
            return "float";
        case TYPE_BOOL:
            return "bool";
        case TYPE_CLASS:
            return "class";
    }
    return "";
}
