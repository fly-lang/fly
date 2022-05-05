//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/GlobalVarParser.cpp - GlobalVar Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/GlobalVarParser.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTStmt.h"

using namespace fly;

/**
 * Parse a GlobalVar
 * @return true on Success or false on Error
 */
ASTGlobalVar *GlobalVarParser::Parse(Parser *P, ASTType *Type, VisibilityKind &Visibility, bool &Constant) {
    assert(P->Tok.isAnyIdentifier() && "Tok must be an Identifier");

    IdentifierInfo *Id = P->Tok.getIdentifierInfo();
    llvm::StringRef Name = Id->getName();
    SourceLocation Loc = P->Tok.getLocation();

    ASTGlobalVar *GlobalVar = new ASTGlobalVar(Loc, P->Node, Type, Name.str(), Visibility, Constant);

    // Parsing =
    P->ConsumeToken();
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();

        ASTValue *Val = P->ParseValue(Type);
        if (Val) {
            GlobalVar->setExpr(new ASTValueExpr(Val));
        }
    }

    return GlobalVar;
}
