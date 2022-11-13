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
#include "AST/ASTParams.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTExpr.h"
#include "Sema/SemaBuilder.h"
#include "Basic/Debug.h"

#include <vector>

using namespace fly;

/**
 * FunctionParser Constructor
 * @param P
 * @param FuncName
 * @param FuncNameLoc
 */
FunctionParser::FunctionParser(Parser *P, ASTTopScopes *Scopes, ASTType *Type, bool isHeader) : P(P) {
    assert(P->Tok.isAnyIdentifier() && "Tok must be an Identifier");

    FLY_DEBUG_MESSAGE("FunctionParser", "FunctionParser", Logger()
                                                    .Attr("Scopes", Scopes)
                                                    .Attr("Type", Type)
                                                    .Attr("isHeader", isHeader).End());

    IdentifierInfo *Id = P->Tok.getIdentifierInfo();
    llvm::StringRef Name = Id->getName();
    const SourceLocation Loc = P->Tok.getLocation();
    P->ConsumeToken();

    Function = P->Builder.CreateFunction(P->Node, Loc, Type, Name.str(), Scopes);
    Success = ParseParams() && !isHeader && ParseBody();
}

/**
 * Parse Parameters
 * @return true on Success or false on Error
 */
bool FunctionParser::ParseParams() {
    FLY_DEBUG("FunctionParser", "ParseParams");

    if (P->Tok.is(tok::l_paren)) { // parse start of function ()
        P->ConsumeParen(); // consume l_paren
    }

    if (P->Tok.is(tok::r_paren)) {
        P->ConsumeParen();
        return true; // end
    }

    // Parse Params as Decl in Function Definition
    return ParseParam();
}

/**
 * Parse a single Function Param
 * @return true on Success or false on Error
 */
bool FunctionParser::ParseParam() {
    FLY_DEBUG("FunctionParser", "ParseParams");

    bool Success = true;

    // Var Constant
    bool Const = P->isConst();

    // Var Type
    ASTType *Type = P->ParseType();
    if (Type) {

        // Var Name
        const StringRef Name = P->Tok.getIdentifierInfo()->getName();
        const SourceLocation IdLoc = P->Tok.getLocation();
        P->ConsumeToken();

        ASTParam *Param = P->Builder.CreateParam(Function, IdLoc, Type, Name.str(), Const);

        ASTExpr *Expr;
        // Parse assignment =
        if (P->Tok.is(tok::equal)) {
            P->ConsumeToken();

            // Start Parsing
            if (P->isValue()) {
                ASTValue *Val = P->ParseValue();
                Expr = P->Builder.CreateExpr(Param, Val);
            }
        } else {
            Expr = P->Builder.CreateExpr(Param); // ASTEmptyExpr
        }

        if (Success && P->Builder.AddParam(Param)) {

            if (P->Tok.is(tok::comma)) {
                P->ConsumeToken();
                return ParseParam();
            }

            if (P->Tok.is(tok::r_paren)) {
                P->ConsumeParen();
                return true; // end
            }
        }
    }

    P->Diag(P->Tok.getLocation(), diag::err_func_param);
    return false;
}

/**
 * Parse Function Body
 * @return true on Success or false on Error
 */
bool FunctionParser::ParseBody() {
    FLY_DEBUG("FunctionParser", "ParseBody");

    if (P->isBlockStart()) {
        bool Success = P->ParseBlock(Function->Body) && P->isBraceBalanced();
        P->ClearBlockComment(); // Clean Block comments for not using them for top definition
        return Success;
    }

    return false;
}

ASTFunction *FunctionParser::Parse(Parser *P, ASTTopScopes *Scopes, ASTType *Type, bool isHeader) {
    FLY_DEBUG_MESSAGE("FunctionParser", "Parse", Logger()
            .Attr("Scopes", Scopes)
            .Attr("Type", Type)
            .Attr("isHeader", isHeader).End());
    FunctionParser *FP = new FunctionParser(P, Scopes, Type, isHeader);
    return FP->Function;
}
