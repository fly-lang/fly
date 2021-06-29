//
// Created by marco on 4/24/21.
//

#include "Parser/GlobalVarParser.h"
#include "AST/Stmt.h"

using namespace fly;

GlobalVarParser::GlobalVarParser(Parser *P, TypeBase *TyDecl, const StringRef &VarName,
                                 SourceLocation &VarNameLoc) :
        P(P), TyDecl(TyDecl), Name(VarName), Location(VarNameLoc) {
}

bool GlobalVarParser::Parse() {

    Var = new GlobalVarDecl(P->AST, Location, TyDecl, Name);

    // Parsing =
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();

        ValueExpr *Ex = P->ParseValueExpr();
        if (Ex) {
            Var->Expression = new GroupExpr();
            Var->Expression->Group.push_back(Ex);
        }
    }

    return true;
}

GlobalVarDecl *GlobalVarParser::getVar() const {
    return Var;
}

Expr *GlobalVarParser::getVal() const {
    return Val;
}
