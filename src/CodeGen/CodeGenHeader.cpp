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
#include "AST/ASTModule.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTIdentity.h"
#include "AST/ASTParam.h"
#include "AST/ASTScopes.h"
#include "AST/ASTEnum.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTEnumEntry.h"
#include "Basic/Diagnostic.h"
#include "Basic/CodeGenOptions.h"
#include "Basic/Debug.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"

#include <string>

using namespace fly;

std::string getParameters(ASTFunctionBase * Function) {
    llvm::Twine Params;
    for (auto &Param : Function->getParams()) {
        Params.concat(Param->getType()->print()).concat(" ").concat(Param->getName()).concat(", ");
    }

    const std::string &ParamsStr = Params.str();
    if (ParamsStr.size() >= 2) {
        ParamsStr.substr(0, ParamsStr.size()-2); // remove ", " from last param
    }
    return ParamsStr;
}

void CodeGenHeader::CreateFile(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts, ASTModule &AST) {
    FLY_DEBUG("CodeGenHeader", "GenerateFile");
    std::string FileHeader = AST.getName() + ".h";
    llvm::StringRef FileName = llvm::sys::path::filename(FileHeader);
    FLY_DEBUG_MESSAGE("CodeGenHeader", "GenerateFile","FileName=" << FileName);

    // generate namespace
    llvm::Twine Header = llvm::Twine("namespace ").concat(AST.getNameSpace()->getName()).concat("\n\n");

    // generate global var declarations
    for (auto GlobalVar : AST.getGlobalVars()) {
        if (GlobalVar->getVisibility() == ASTVisibilityKind::V_PUBLIC) {
            Header.concat(GlobalVar->getType()->print())
            .concat(GlobalVar->getName())
            .concat("\n\n");
        }
    }

    // generate function declarations
    for (auto &Function : AST.getFunctions()) {
        if (Function->getVisibility() == ASTVisibilityKind::V_PUBLIC) {
            Header.concat(Function->getReturnType()->print())
                    .concat(Function->getName())
                    .concat("(");
            std::string ParamsStr = getParameters(Function);
            Header.concat(ParamsStr).concat(")").concat("\n\n");
        }
    }

    // generate Identity Header: class, enum
    for (auto Identity : AST.getIdentities()) {
        Header.concat(Identity->getName()).concat("{\n");
        if (Identity->getIdentityKind() == ASTIdentityKind::ID_CLASS) {
            ASTClass* Class = (ASTClass *) Identity;
            for (auto Attribute : Class->getAttributes()) {
                Header.concat(Attribute->getType()->print()).concat(" ")
                    .concat(Attribute->getName()).concat("\n\n");
            }
            for (auto Constructor: Class->getConstructors()) {
                if (Constructor->getVisibility() == ASTVisibilityKind::V_PUBLIC) {
                    Header.concat(Constructor->getReturnType()->print())
                            .concat(Constructor->getName())
                            .concat("(");
                    std::string ParamsStr = getParameters(Constructor);
                    Header.concat(ParamsStr).concat(")").concat("\n\n");
                }
            }
            for (auto &Method: Class->getMethods()) {
                if (Method->getVisibility() == ASTVisibilityKind::V_PUBLIC) {
                    Header.concat(Method->getReturnType()->print())
                            .concat(Method->getName())
                            .concat("(");
                    std::string ParamsStr = getParameters(Method);
                    Header.concat(ParamsStr).concat(")").concat("\n\n");
                }
            }
        } else if (Identity->getIdentityKind() == ASTIdentityKind::ID_ENUM) {
            ASTEnum* Enum = (ASTEnum *) Identity;
            for (auto &EnumEntry : Enum->getEntries()) {
                Header.concat(EnumEntry->getName()).concat("\n\n"); // TODO add value
            }
        }
        Header.concat("}\n\n");
    }

    int FD;
    const std::error_code &EC = llvm::sys::fs::openFileForWrite(FileName, FD,
                                                                llvm::sys::fs::CD_CreateAlways,
                                                                llvm::sys::fs::F_None);
    if (EC) {
        Diags.Report(diag::err_generate_header) << EC.message();
    }

    llvm::raw_fd_ostream file(FD, true);

    // Write the data.
//    StringRef StrRef(Header);
//    file.write(Header.data(), StrRef.size());
    file << Header;
    // Header.print(file);
    file.close();
}

