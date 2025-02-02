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
#include "AST/ASTBlockStmt.h"
#include "Basic/Debug.h"
#include "Sema/ASTBuilder.h"

using namespace fly;

ParserFunction::ParserFunction(Parser *P) : P(P) {

}

ASTFunction *ParserFunction::Parse(Parser *P, SmallVector<ASTScope *, 8> Scopes, ASTTypeRef *Type) {
    ParserFunction *PF = new ParserFunction(P);

    assert(P->Tok.isAnyIdentifier() && "Tok must be an Identifier");
    StringRef Name = P->Tok.getIdentifierInfo()->getName();
    const SourceLocation &Loc = P->ConsumeToken();
    SmallVector<ASTVar *, 8> Params = PF->ParseParams();
    ASTFunction *Function = P->Builder.CreateFunction(P->Module, Loc, Type, Name, Scopes, Params, nullptr);
    ASTBlockStmt *Body = P->isBlockStart() ? PF->ParseBody(Function) : nullptr;
    return Function;
}

/**
 * ParseModule Function Body
 * @return true on Success or false on Error
 */
ASTBlockStmt *ParserFunction::ParseBody(ASTFunction *F) {
    FLY_DEBUG("FunctionParser", "ParseBody");
    assert(P->isBlockStart() && "Block Start");
    ASTBlockStmt *Block = P->Builder.CreateBlockStmt(P->Tok.getLocation());
    ASTBlockStmt *Body = P->Builder.CreateBody(F, Block);
    P->ParseBlock(Body);
    return Body;
}


/**
 * ParseModule Parameters
 * @return true on Success or false on Error
 */
llvm::SmallVector<ASTVar *, 8> ParserFunction::ParseParams() {
    FLY_DEBUG("FunctionParser", "ParseParams");
    assert(P->Tok.is(tok::l_paren) && "Tok must be an Identifier");
    P->ConsumeParen(); // consume l_paren

    llvm::SmallVector<ASTVar *, 8> Params;

    while (true) {
        // Check for closing parenthesis (end of parameter list)
        if (P->Tok.is(tok::r_paren)) {
            P->ConsumeParen();
            break;
        }

        // Parse a parameter
        ASTVar *Param = ParseParam();
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
            P->Diag(P->Tok.getLocation(), diag::err_parse_expected_comma_or_rparen);
        }
    }

    return Params;
}

/**
 * ParseModule a single Function Param
 * @return true on Success or false on Error
 */
ASTVar *ParserFunction::ParseParam() {
    FLY_DEBUG("FunctionParser", "ParseParam");

    // Parse Scopes
    llvm::SmallVector<ASTScope *, 8> Scopes = P->ParseScopes();

    // Var Type
    ASTType *Type = P->ParseTypeRef();
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

    ASTVar *Param = P->Builder.CreateParam(Loc, Type, Name, Scopes, Value);
    return Param;
}
