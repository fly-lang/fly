//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaCall.cpp - function call semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaCall.h"
#include "Sema/SemaLocalVar.h"
#include "Sema/SemaVisitor.h"
#include "Sema/SemaType.h"
#include "Sema/SemaFunctionBase.h"
#include "Basic/Logger.h"

#include <AST/ASTCall.h>

using namespace fly;

SemaCall::SemaCall(ASTCall &AST, SemaType *Type) : SemaExpr(SemaKind::CALL, Type), AST(AST) {
}

ASTCall &SemaCall::getAST() const {
	return AST;
}

SemaFunctionBase *SemaCall::getFunction() const {
	return Function;
}

SemaError *SemaCall::getErrorHandler() const {
	return ErrorHandler;
}

void SemaCall::setErrorHandler(SemaError *ErrorHandler) {
	this->ErrorHandler = ErrorHandler;
}

bool SemaCall::isNew() const {
	return AST.getCallKind() >= ASTCallKind::CALL_NEW;
}

SemaLocalVar *SemaCall::getOutVar() const {
	return OutVar;
}

llvm::SmallVector<SemaExpr *, 8> &SemaCall::getArgs() {
	return Args;
}

void SemaCall::addArg(SemaExpr *Arg) {
	Args.push_back(Arg);
}

CodeGenExpr * SemaCall::getCodeGen() const {
	return CodeGen;
}

void SemaCall::setCodeGen(CodeGenExpr *CodeGen) {
	this->CodeGen = CodeGen;
}

void SemaCall::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaCall::str() const {
	return Logger("SemaCall")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("AST", AST.str())
		.Attr("Type", Type)
		.Attr("Function", Function)
		.Attr("Args", Args)
		.End();
}

