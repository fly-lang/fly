//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenHeader.cpp - Header Generator implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenHeader.h"
#include "Sema/SemaNameSpace.h"
#include "Sema/SemaGlobalVar.h"
#include "Sema/SemaFunction.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaEnumType.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"

#include <string>

using namespace fly;

std::string getParameters(ASTFunction *Function) {
	llvm::Twine Params;
	for (auto Param : Function->getParams()) {
		//SemaTypeKind Kind = Param->getTypeRef()->getDef()->getKind();
		//Params.concat(Kind).concat(" ").concat(Param->getName()).concat(", ");
	}

	const std::string &ParamsStr = Params.str();
	if (ParamsStr.size() >= 2) {
		ParamsStr.substr(0, ParamsStr.size() - 2); // remove ", " from last param
	}
	return ParamsStr;
}

void CodeGenHeader::CreateFile(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts, SemaNameSpace &NameSpace) {
	FLY_DEBUG_START("CodeGenHeader", "GenerateFile");
	llvm::Twine FileHeader = llvm::Twine(NameSpace.getName()).concat(".h");
	llvm::StringRef FileName = llvm::sys::path::filename(FileHeader.str());
	FLY_DEBUG_START_MSG("CodeGenHeader", "GenerateFile", "FileName=" << FileName);

	// generate namespace
	llvm::Twine Header = llvm::Twine("namespace ").concat(NameSpace.getName()).concat("\n\n");

	// generate global var declarations
	for (auto SemaVar : NameSpace.getGlobalVars()) {
		ASTVar *GlobalVar = SemaVar.getValue()->getAST();
		if (GlobalVar->getVisibility() == ASTModifierKind::V_PUBLIC) {
			Header.concat(GlobalVar->getTypeRef()->print())
			      .concat(GlobalVar->getName())
			      .concat("\n\n");
		}
	}

	// generate function declarations
	for (auto &SemaFunc : NameSpace.getFunctions()) {
		ASTFunction *Function = SemaFunc->getFunction();
		if (Function->getVisibility() == ASTModifierKind::V_PUBLIC) {
			// Functions are implicitly void - no return type to print
			Header.concat(Function->getName())
			      .concat("(");
			std::string ParamsStr = getParameters(Function);
			Header.concat(ParamsStr).concat(")").concat("\n\n");
		}
	}

	// generate Identity Header: class, enum
	for (auto SemaClassEntry : NameSpace.getClasses()) {
		ASTClass *Class = SemaClassEntry.getValue()->getAST();
		Header.concat(Class->getName()).concat("{\n");
		for (auto Attribute : Class->getAttributes()) {
			Header.concat(Attribute->getType()->print()).concat(" ")
			      .concat(Attribute->getName()).concat("\n\n");
		}
		for (auto Constructor : Class->getConstructors()) {
			if (Constructor->getVisibility() == ASTModifierKind::V_PUBLIC) {
				// Constructors are implicitly void - no return type to print
				Header.concat(Constructor->getName())
				      .concat("(");
				std::string ParamsStr = getParameters(Constructor);
				Header.concat(ParamsStr).concat(")").concat("\n\n");
			}
		}
		for (auto &Method : Class->getMethods()) {
			if (Method->getVisibility() == ASTModifierKind::V_PUBLIC) {
				// Methods are implicitly void - no return type to print
				Header.concat(Method->getName())
				      .concat("(");
				std::string ParamsStr = getParameters(Method);
				Header.concat(ParamsStr).concat(")").concat("\n\n");
			}
		}
		Header.concat("}\n\n");
	}

	for (auto SemaEnum : NameSpace.getEnums()) {
		ASTEnum *Enum = SemaEnum.getValue()->getAST();
		Header.concat(Enum->getName()).concat("{\n");
		for (auto &EnumEntry : Enum->getEntries()) {
			Header.concat(EnumEntry->getName()).concat("\n\n"); // TODO add value
		}
	}

	int FD;
	const std::error_code &EC = llvm::sys::fs::openFileForWrite(
		FileName, FD,
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