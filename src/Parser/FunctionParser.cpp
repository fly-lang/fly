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
#include "AST/ASTFunctionCall.h"
#include "Sema/SemaBuilder.h"

#include <vector>

using namespace fly;

/**
 * FunctionParser Constructor
 * @param P
 * @param FuncName
 * @param FuncNameLoc
 */
FunctionParser::FunctionParser(Parser *P, VisibilityKind &Visibility, ASTType *Type, bool isHeader) : P(P) {
    assert(P->Tok.isAnyIdentifier() && "Tok must be an Identifier");

    IdentifierInfo *Id = P->Tok.getIdentifierInfo();
    llvm::StringRef Name = Id->getName();
    const SourceLocation Loc = P->Tok.getLocation();
    P->ConsumeToken();

    Function = P->Builder.CreateFunction(P->Node, Loc, Type, Name.str(), Visibility);
    ParseParams() && !isHeader && ParseBody();
}

/**
 * Parse Parameters
 * @return true on Success or false on Error
 */
bool FunctionParser::ParseParams() {
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
    bool Success = true;

    // Var Constant
    bool Const = P->isConst();

    // Var Type
    ASTType *Type = P->ParseType(false);
    if (Type) {

        // Var Name
        const StringRef Name = P->Tok.getIdentifierInfo()->getName();
        const SourceLocation IdLoc = P->Tok.getLocation();
        P->ConsumeToken();

        ASTParam *Param = P->Builder.CreateParam(IdLoc, Type, Name.str(), Const);

        // Parse assignment =
        if (P->Tok.is(tok::equal)) {
            P->ConsumeToken();

            // Start Parsing
            if (P->isValue()) {
                ASTValue *Val = P->ParseValue();
                Val && P->Builder.setVarExpr(Param, P->Builder.CreateExpr(Val));
            }
        }

        if (Success && P->Builder.AddFunctionParam(Function, Param)) {

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
    if (P->Tok.is(tok::l_brace)) {
        P->ConsumeBrace();

        bool Success = P->ParseInnerBlock(Function->Body) && P->isBraceBalanced();
        P->ClearBlockComment(); // Clean Block comments for not using them for top definition
        return Success;
    }

    return false;
}

ASTFunction *FunctionParser::Parse(Parser *P, VisibilityKind &Visibility, ASTType *Type, bool isHeader) {
    FunctionParser *FP = new FunctionParser(P, Visibility, Type, isHeader);
    return FP->Function;
}
