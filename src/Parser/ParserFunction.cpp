//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/FunctionParser.cpp - Function Declaration and Call Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "Parser/ParserFunction.h"
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
ASTParam *ParserFunction::ParseParam(Parser *P) {
    FLY_DEBUG("FunctionParser", "ParseParam");

    // Parse Scopes
    llvm::SmallVector<ASTScope *, 8> Scopes = P->ParseScopes();

    // Var Type
    ASTType *Type = P->ParseType();
    if (!Type) {
        P->Diag(diag::err_parser_invalid_type);
        return nullptr;
    }

    // Var Name
    const StringRef Name = P->Tok.getIdentifierInfo()->getName();
    const SourceLocation &Loc = P->ConsumeToken();

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
llvm::SmallVector<ASTParam *, 8> ParserFunction::ParseParams(Parser *P) {
    FLY_DEBUG("FunctionParser", "ParseParams");
    assert(P->Tok.is(tok::l_paren) && "Tok must be an Identifier");
    P->ConsumeParen(); // consume l_paren

    llvm::SmallVector<ASTParam *, 8> Params;
    ASTParam *Param;
    while (P->Tok.isNot(tok::r_paren) && P->Tok.isNot(tok::eof) && (Param = ParseParam(P))) {
        Params.push_back(Param);
        if (P->Tok.is(tok::comma)) {
            P->ConsumeToken();
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
ASTBlockStmt *ParserFunction::ParseBody(Parser *P) {
    FLY_DEBUG("FunctionParser", "ParseBody");
    assert(P->isBlockStart() && "Block Start");
    ASTBlockStmt *Body = P->Builder.CreateBlockStmt(P->Tok.getLocation());
    P->ParseBlock(Body, true);
    return Body;
}
