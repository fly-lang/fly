//
// Created by marco on 4/24/21.
//

#include "Parser/GlobalVarParser.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTStmt.h"

using namespace fly;

GlobalVarParser::GlobalVarParser(Parser *P, ASTType *TyDecl, const StringRef &VarName,
                                 SourceLocation &VarNameLoc) :
        P(P), TyDecl(TyDecl), Name(VarName), Location(VarNameLoc) {
}

bool GlobalVarParser::Parse() {

    Var = new ASTGlobalVar(P->AST, Location, TyDecl, Name);

    // Parsing =
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();

        ASTValueExpr *Ex = P->ParseValueExpr();
        if (Ex) {
            Var->setExpr(Ex);
        }
    }

    return true;
}

ASTGlobalVar *GlobalVarParser::getVar() const {
    return Var;
}

ASTExpr *GlobalVarParser::getVal() const {
    return Val;
}
