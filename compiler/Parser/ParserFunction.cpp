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
#include "AST/ASTVar.h"
#include "Basic/Debug.h"
#include "AST/ASTBuilder.h"

#include <Parser/ParserExpr.h>

using namespace fly;

/**
 * ParseModule Function Body
 * @return true on Success or false on Error
 */
ASTBlockStmt *ParserFunction::ParseBody(Parser *P, ASTFunction *F) {
    FLY_DEBUG_SCOPE("ParserFunction", "ParseBody");
    assert(P->isBlockStart() && "Block Start");
    ASTBlockStmt *Block = ASTBuilder::CreateBlockStmt(P->Tok.getLocation());
    ASTBlockStmt *Body = ASTBuilder::CreateBody(F, Block);
    P->ParseBlock(Body);
    return Body;
}


/**
 * ParseModule Parameters
 * @return true on Success or false on Error
 */
llvm::SmallVector<ASTParam *, 8> ParserFunction::ParseParams(Parser *P) {
    FLY_DEBUG_SCOPE("ParserFunction", "ParseParams");
    assert(P->Tok.is(tok::l_paren) && "Tok must be an Identifier");
    P->ConsumeParen(); // consume l_paren

    llvm::SmallVector<ASTParam *, 8> Params;

    while (true) {
        // Check for closing parenthesis (end of parameter list)
        if (P->Tok.is(tok::r_paren)) {
            P->ConsumeParen();
            break;
        }

        // Parse a parameter
        ASTParam *Param = ParseParam(P);
        if (Param == nullptr) {
            // Handle error: Invalid parameter syntax
            P->Diag(P->Tok.getLocation(), diag::err_parser_invalid_param);
            break;
        }

        // Add the parsed parameter to the list
        Params.push_back(Param);

        // Check for a comma (',') to separate parameters
        if (P->Tok.is(tok::comma)) {
            P->ConsumeToken(); // Consume the comma and continue
        } else if (P->Tok.is(tok::r_paren)) {
            P->ConsumeParen();
            break; // End of parameter list
        } else {
            // Handle error: Unexpected token
            P->Diag(P->Tok.getLocation(), diag::err_parser_expected_comma_or_rparen);
        }
    }

    return Params;
}

/**
 * ParseModule a single Function Param
 * @return true on Success or false on Error
 */
ASTParam *ParserFunction::ParseParam(Parser *P) {
    FLY_DEBUG_SCOPE("ParserFunction", "ParseParam");

    // Parse Modifiers
    llvm::SmallVector<ASTModifier *, 8> Modifiers = P->ParseModifiers();

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
            ParserExpr PE(P);
            Value = PE.ParseValue();
        }
    }

    ASTParam *Param = ASTBuilder::CreateParam(Loc, Type, Name, Modifiers, Value);
    return Param;
}
