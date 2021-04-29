//
// Created by marco on 4/27/21.
//

#include <Parser/FunctionParser.h>

using namespace fly;

FunctionParser::FunctionParser(Parser *P, TypeDecl *RetTyDecl, const StringRef &FuncName,
                                    SourceLocation &FuncNameLoc)
                                    : P(P), RetTyDecl(RetTyDecl), FuncName(FuncName), FuncNameLoc(FuncNameLoc) {

}

bool FunctionParser::Parse() {
    if (!P->Tok.is(tok::l_paren)) {
        P->Diag(P->Tok, diag::err_func_paren_start);
        return false;
    }

    Function = new FunctionDecl(FuncNameLoc, RetTyDecl, FuncName);
    P->ConsumeParen();
    if (ParseParameters(true)) {
        return ParseBody();
    }
    return false;
}

bool FunctionParser::ParseParameters(bool isStart) {
    if (P->Tok.is(tok::r_paren)) {
        P->ConsumeParen();
        return true;
    } else if (!isStart && P->Tok.is(tok::comma)) {
        P->ConsumeToken();
    }

    // Parse Param
    VarDecl *Var = P->ParseVarDecl();
    if (Var) {
        Function->Params->Vars.push_back(Var);
        return ParseParameters();
    } else {
        P->Diag(P->Tok.getLocation(), diag::err_func_param);
        return false;
    }
}

bool FunctionParser::ParseBody() {
    Function->Body = new Stmt(P->Tok.getLocation(), Function->Params->Vars);
    return P->ParseStmt(Function->Body, true);
}
