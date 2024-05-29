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
    for (auto &Entry : AST.getGlobalVars()) {
        ASTGlobalVar *GlobalVar = Entry.getValue();
        if (GlobalVar->getScopes()->getVisibility() == ASTVisibilityKind::V_PUBLIC) {
            Header.concat(GlobalVar->getType()->print())
            .concat(GlobalVar->getName())
            .concat("\n\n");
        }
    }

    // generate function declarations
    for (auto &StrMapEntry : AST.getFunctions()) {
        for (auto &IntMap : StrMapEntry.getValue()) {
            for (auto &Function: IntMap.second) {
                if (Function->getScopes()->getVisibility() == ASTVisibilityKind::V_PUBLIC) {
                    Header.concat(Function->getType()->print())
                            .concat(Function->getName())
                            .concat("(");
                    std::string ParamsStr = getParameters(Function);
                    Header.concat(ParamsStr).concat(")").concat("\n\n");
                }
            }
        }
    }

    // generate Identity Header: class, enum
    for (auto &Entry : AST.getIdentities()) {
        ASTIdentity *Identity = Entry.getValue();
        Header.concat(Identity->getName()).concat("{\n");
        if (Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS) {
            ASTClass* Class = (ASTClass *) Identity;
            for (auto &AttrEntry : Class->getAttributes()) {
                Header.concat(AttrEntry.getValue()->getType()->print()).concat(" ")
                    .concat(AttrEntry.getValue()->getName()).concat("\n\n");
            }
            for (auto &IntMap: Class->getConstructors()) {
                for (auto &Constructor: IntMap.second) {
                    if (Constructor->getScopes()->getVisibility() == ASTVisibilityKind::V_PUBLIC) {
                        Header.concat(Constructor->getType()->print())
                                .concat(Constructor->getName())
                                .concat("(");
                        std::string ParamsStr = getParameters(Constructor);
                        Header.concat(ParamsStr).concat(")").concat("\n\n");
                    }
                }
            }
            for (auto &Map: Class->getMethods()) {
                for (auto &Entry: Map.second) {
                    for (auto Method: Entry.second) {
                        if (Method->getScopes()->getVisibility() == ASTVisibilityKind::V_PUBLIC) {
                            Header.concat(Method->getType()->print())
                                    .concat(Method->getName())
                                    .concat("(");
                            std::string ParamsStr = getParameters(Method);
                            Header.concat(ParamsStr).concat(")").concat("\n\n");
                        }
                    }
                }
            }
        } else if (Identity->getTopDefKind() == ASTTopDefKind::DEF_ENUM) {
            ASTEnum* Enum = (ASTEnum *) Identity;
            for (auto &EnumEntry : Enum->getEntries()) {
                Header.concat(EnumEntry.getKey()).concat("\n\n");
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

