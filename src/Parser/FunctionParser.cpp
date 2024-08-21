//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/FunctionParser.cpp - Function Declaration and Call Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "Parser/FunctionParser.h"
#include "AST/ASTParam.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTExpr.h"
#include "AST/ASTBlockStmt.h"
#include "Sema/SemaBuilder.h"
#include "Basic/Debug.h"

#include <vector>

using namespace fly;

/**
 * ParseModule a single Function Param
 * @return true on Success or false on Error
 */
ASTParam *FunctionParser::ParseParam(Parser *P) {
    FLY_DEBUG("FunctionParser", "ParseParam");

    // Var Constant
    bool Const = P->isConst();

    // Var Type
    ASTType *Type = nullptr;
    if (!P->ParseType(Type)) {
        P->Diag(diag::err_parser_invalid_type);
        return nullptr;
    }

    // Var Name
    const StringRef Name = P->Tok.getIdentifierInfo()->getName();
    const SourceLocation Loc = P->Tok.getLocation();
    P->ConsumeToken();

    // Parse Scopes
    llvm::SmallVector<ASTScope *, 8> Scopes = P->ParseScopes();

    // Parse assignment =
    ASTValue *Value = nullptr;
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();

        // Start Parsing
        if (P->isValue()) {
            Value = P->ParseValue();
        }
    }

    ASTParam *Param = P->Builder.CreateParam(Loc, Type, Name, Scopes, Value);
    return Param;
}

/**
 * ParseModule Parameters
 * @return true on Success or false on Error
 */
llvm::SmallVector<ASTParam *, 8> FunctionParser::ParseParams(Parser *P) {
    FLY_DEBUG("FunctionParser", "ParseParams");

    if (P->Tok.is(tok::l_paren)) { // parse start of function ()
        P->ConsumeParen(); // consume l_paren
    }

    llvm::SmallVector<ASTParam *, 8> Params;
    ASTParam *Param;
    while ((Param = ParseParam(P))) {
        Params.push_back(Param);
        if (P->Tok.isNot(tok::comma)) {
            break;
        }
    }

    if (P->Tok.is(tok::r_paren)) {
        P->ConsumeParen();
    } else {
        // FIXME Error desc
        P->Diag(P->Tok.getLocation(), diag::err_parser_generic);
    }

    return Params;
}

/**
 * ParseModule Function Body
 * @return true on Success or false on Error
 */
ASTBlockStmt *FunctionParser::ParseBody(Parser *P) {
    FLY_DEBUG("FunctionParser", "ParseBody");

    ASTBlockStmt *Body = nullptr;
    if (P->isBlockStart()) {
        Body = P->Builder.CreateBlockStmt(SourceLocation());
        P->ParseBlock(Body) && P->isBraceBalanced();
    }

    return Body;
}
