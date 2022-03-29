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
 * GlobalVarParser Constructor
 * @param P
 * @param TyDecl
 * @param VarName
 * @param VarNameLoc
 */
GlobalVarParser::GlobalVarParser(Parser *P, ASTType *TyDecl) : P(P), Type(TyDecl) {
}

/**
 * Parse a GlobalVar
 * @return true on Success or false on Error
 */
bool GlobalVarParser::Parse() {
    assert(P->Tok.isAnyIdentifier() && "Tok must be an Identifier");

    IdentifierInfo *Id = P->Tok.getIdentifierInfo();
    llvm::StringRef Name = Id->getName();
    SourceLocation Loc = P->Tok.getLocation();

    AST = new ASTGlobalVar(Loc, P->AST, Type, Name.str());

    // Add Comment to AST
    if (!P->BlockComment.empty()) {
        AST->setComment(P->BlockComment);
        P->ClearBlockComment(); // Clear for next use
    }

    // Parsing =
    P->ConsumeToken();
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();

        ASTValue *Val = P->ParseValue();
        if (Val != nullptr) {
            AST->setExpr(new ASTValueExpr(Val));
        }
    }

    return true;
}

/**
 * Get Var
 * @return Var
 */
ASTGlobalVar *GlobalVarParser::getAST() const {
    return AST;
}
