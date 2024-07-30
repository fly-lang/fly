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


using namespace fly;

ASTCall::ASTCall(const SourceLocation &Loc, llvm::StringRef Name) : ASTIdentifier(Loc, Name, ASTIdentifierKind::REF_CALL) {

}

const ASTVar *ASTCall::getErrorHandler() const {
    return ErrorHandler;
}

llvm::SmallVector<ASTArg *, 8> ASTCall::getArgs() const {
    return Args;
}

ASTFunctionBase *ASTCall::getDef() const {
    return Def;
}

ASTCallKind ASTCall::getCallKind() const {
    return CallKind;
}

ASTMemoryKind ASTCall::getMemoryKind() const {
    return MemoryKind;
}

std::string ASTCall::str() const {
    return Logger("ASTFunctionCall").
            AttrList("Args", Args).
            Attr("Def", Def).
            End();
}
