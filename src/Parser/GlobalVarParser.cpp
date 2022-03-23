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
GlobalVarParser::GlobalVarParser(Parser *P, ASTType *TyDecl, const StringRef &VarName,
                                 SourceLocation &VarNameLoc) :
        P(P), Type(TyDecl), Name(VarName), Location(VarNameLoc) {
}

/**
 * Parse a GlobalVar
 * @return true on Success or false on Error
 */
bool GlobalVarParser::Parse() {
    bool Success = true;
    Var = new ASTGlobalVar(Location, P->AST, Type, Name.str());

    // Parsing =
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();

        ASTValue *Val = P->ParseValue();
        if (Val != nullptr) {
            Var->setExpr(new ASTValueExpr(Val));
        }
    }

    return Success;
}

/**
 * Get Var
 * @return Var
 */
ASTGlobalVar *GlobalVarParser::getVar() const {
    return Var;
}

/**
 * Get Val
 * @return Val
 */
ASTExpr *GlobalVarParser::getVal() const {
    return Val;
}
