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
FunctionParser::FunctionParser(Parser *P, ASTFunctionBase *Function) : P(P), Function(Function) {
    assert(Function && "Function must be initialized");

    FLY_DEBUG_MESSAGE("FunctionParser", "FunctionParser", Logger()
                        .Attr("Function", Function).End());
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

    // Var Constant
    bool Const = P->isConst();

    // Var Type
    ASTType *Type = nullptr;
    if (P->ParseType(Type)) {

        // Var Name
        const StringRef Name = P->Tok.getIdentifierInfo()->getName();
        const SourceLocation IdLoc = P->Tok.getLocation();
        P->ConsumeToken();

        ASTScopes *Scopes = SemaBuilder::CreateScopes();
        P->ParseScopes(Scopes);

        ASTParam *Param = SemaBuilder::CreateParam(Function, IdLoc, Type, Name, Scopes);

        // Parse assignment =
        if (P->Tok.is(tok::equal)) {
            P->ConsumeToken();

            // Start Parsing
            if (P->isValue()) {
                ASTValue *Val = P->ParseValue();
                Param->setDefaultValue(Val);
            }
        }

        if (P->Builder.AddParam(Param)) {

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
        Function->Body = P->Builder.CreateBody(Function);
        return P->ParseBlock(Function->Body) && P->isBraceBalanced();
    }

    return true;
}

bool FunctionParser::Parse(Parser *P, ASTFunctionBase *Function) {
    FLY_DEBUG_MESSAGE("FunctionParser", "Parse", Logger()
            .Attr("Function", Function).End());
    FunctionParser *FP = new FunctionParser(P, Function);
    bool Success = FP->ParseParams() && FP->ParseBody();
    return Success;
}
