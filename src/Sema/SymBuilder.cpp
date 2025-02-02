//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymBuilder.cpp - The Symbolic Table Builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Sema.h"
#include "Sym/SymTable.h"
#include "Sym/SymModule.h"
#include "Sym/SymNameSpace.h"
#include "Sema/SymBuilder.h"
#include "Basic/Debug.h"
#include "Sym/SymClass.h"
#include "Sym/SymEnum.h"
#include "Sym/SymFunction.h"
#include "Sym/SymGlobalVar.h"
#include "Sym/SymType.h"

#include <AST/ASTClass.h>
#include <AST/ASTEnum.h>
#include <AST/ASTFunction.h>
#include <AST/ASTVar.h>
#include <AST/ASTModule.h>
#include <Sym/SymClassAttribute.h>
#include <Sym/SymClassMethod.h>
#include <Sym/SymEnumEntry.h>
#include <Sym/SymComment.h>

using namespace fly;

SymBuilder::SymBuilder(Sema &S) : S(S) {

}

SymTable * SymBuilder::CreateTable() {
	return new SymTable();
}

SymNameSpace *SymBuilder::CreateNameSpace() {
	SymNameSpace *NameSpace = new SymNameSpace();
	S.Table->NameSpaces.insert(std::make_pair<>(NameSpace->getName(), NameSpace));
	S.Table->DefaultNameSpace = NameSpace;
	return NameSpace;
}

SymNameSpace *SymBuilder::CreateNameSpace(llvm::StringRef Name) {
	FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNameSpace", "Name=" << Name);

	SymNameSpace *NameSpace = new SymNameSpace(Name);
	S.Table->NameSpaces.insert(std::make_pair<>(NameSpace->getName(), NameSpace));
	return NameSpace;
}

SymModule * SymBuilder::CreateModule(ASTModule *AST) {
	SymModule *Module = new SymModule(AST);
	S.Table->Modules.insert(std::make_pair(AST->getName(), Module));
	return Module;
}

SymGlobalVar * SymBuilder::CreateGlobalVar(SymModule *Module, ASTVar *AST) {
	SymGlobalVar *GlobalVar = new SymGlobalVar(AST);
	Module->GlobalVars.insert(std::make_pair(AST->getName(), GlobalVar));
	return GlobalVar;
}

SymFunction * SymBuilder::CreateFunction(SymModule *Module, ASTFunction *AST) {
	SymFunction *Function = new SymFunction(AST);
	Module->Functions.insert(std::make_pair(AST->getName(), Function));
	return Function;
}

SymClass * SymBuilder::CreateClass(SymModule *Module, ASTClass *AST) {
	SymClass *Class = new SymClass(AST);
	Module->Classes.insert(std::make_pair(AST->getName(), Class));
	return Class;
}

SymEnum * SymBuilder::CreateEnum(SymModule *Module, ASTEnum *AST) {
	SymEnum *Enum = new SymEnum(AST);
	Module->Enums.insert(std::make_pair(AST->getName(), Enum));
	return Enum;
}

SymClassAttribute * SymBuilder::CreateClassAttribute(SymClass *Class, ASTVar *AST) {
	SymClassAttribute *Attribute = new SymClassAttribute(AST);
	Class->Attributes.insert(std::make_pair(AST->getName(), Attribute));
}

SymClassMethod * SymBuilder::CreateClassFunction(SymClass *Class, ASTFunction *AST) {
	SymClassMethod *Method = new SymClassMethod(AST);
	//Class->Methods.insert(std::make_pair(AST->getName(), Method)); // FIXME
}

SymEnum * SymBuilder::CreateEnumEntry(SymEnum *Enum, ASTVar *AST) {
	SymEnumEntry *Entry = new SymEnumEntry(AST);
	Enum->Entries.insert(std::make_pair(AST->getName(), Entry));
}

SymType * SymBuilder::CreateType(SymTypeKind Kind) {
	SymType *Type = new SymType(Kind);
	return Type;
}

SymTypeInt * SymBuilder::CreateIntType(SymIntTypeKind IntKind) {
	SymTypeInt *Type = new SymTypeInt(IntKind);
	return Type;
}

SymTypeFP * SymBuilder::CreateFPType(SymFPTypeKind FPKind) {
	SymTypeFP *Type = new SymTypeFP(FPKind);
	return Type;
}

SymTypeArray * SymBuilder::CreateArrayType(SymType Type, uint64_t Size) {
	return new SymTypeArray(Type, Size);
}

SymComment * SymBuilder::CreateComment(ASTComment *AST) {
	return new SymComment(AST);
}
