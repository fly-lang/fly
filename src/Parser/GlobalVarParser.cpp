//
// Created by marco on 4/24/21.
//

#include "Parser/GlobalVarParser.h"
#include "AST/Decl.h"

using namespace fly;

GlobalVarParser::GlobalVarParser(Parser *P, TypeDecl *TyDecl, const StringRef &VarName,
                                 SourceLocation &VarNameLoc) :
        P(P), TyDecl(TyDecl), Name(VarName), Location(VarNameLoc) {
}

bool GlobalVarParser::Parse() {

    Var = new GlobalVarDecl(Location, TyDecl, Name);

    // Parsing =
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();

        Expr* Ex = P->ParseExpr();
        if (Ex && Ex->getKind() == EXPR_VALUE) {
            Var->Expression = Ex;
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
