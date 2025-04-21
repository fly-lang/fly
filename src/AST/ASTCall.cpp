//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTCall.cpp - AST Function Call implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTCall.h"
#include "AST/ASTArg.h"
#include "Basic/Logger.h"


using namespace fly;

ASTCall::ASTCall(const SourceLocation &Loc, llvm::StringRef Name) : ASTRef(Loc, Name, ASTRefKind::REF_CALL) {

}

SymErrorHandler *ASTCall::getErrorHandler() const {
    return ErrorHandler;
}

llvm::SmallVector<ASTArg *, 8> ASTCall::getArgs() const {
    return Args;
}

SymFunctionBase *ASTCall::getFunction() const {
    return *Function;
}

ASTCallKind ASTCall::getCallKind() const {
    return CallKind;
}

std::string ASTCall::str() const {
    return Logger("ASTCall").
	Attr("Location", getLocation()).
		Attr("Kind", static_cast<size_t>(getKind())).
            Attr("Args", ASTBase::str(Args)).
            Attr("Def", Function).
            End();
}
