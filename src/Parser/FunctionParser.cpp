//
// Created by marco on 4/27/21.
//

#include <Parser/FunctionParser.h>

using namespace fly;

FunctionParser::FunctionParser(Parser *P, const StringRef &FuncName, SourceLocation &FuncNameLoc)
                                    : P(P), FuncName(FuncName), FuncNameLoc(FuncNameLoc) {

}

bool FunctionParser::ParseRefDecl() {
    Invoke = new FuncRefDecl(FuncNameLoc, FuncName);
    return ParseParameters(true, true);
}

bool FunctionParser::ParseDefinition(TypeBase *TyDecl) {
    Function = new FuncDecl(FuncNameLoc, TyDecl, FuncName);
    if (ParseParameters(true)) {
        return ParseBody();
    }
    return false;
}

bool FunctionParser::ParseParameters(bool isStart, bool isRef) {
    if (isStart && !P->Tok.is(tok::l_paren)) {
        assert("is not a function");
    }

    if (isStart && P->Tok.is(tok::l_paren)) {
        P->ConsumeParen(); // consume l_paren
    }

    if (P->Tok.is(tok::r_paren)) {
        P->ConsumeParen();
        return true;
    } else if (!isStart && P->Tok.is(tok::comma)) {
        P->ConsumeToken();
    }

    // Parse Params as Ref in Function Call
    if (isRef) {
        if (P->Tok.isAnyIdentifier()) {
            VarRef *Var = P->ParseVarRef();
            VarRefExpr *RExpr = new VarRefExpr(P->Tok.getLocation(), Var);
            Invoke->Params->Args.push_back(RExpr);
            return ParseParameters(false, true);
        } else if (P->Tok.is(tok::numeric_constant)) {
            const StringRef V = StringRef(P->Tok.getLiteralData(), P->Tok.getLength());
            SourceLocation Loc = P->ConsumeToken();
            Invoke->Params->Args.push_back(new ValueExpr(Loc, V));
            return ParseParameters(false, true);
        } else if (P->isBoolValue()) {
            StringRef B = P->ParseBoolValue();
            SourceLocation Loc = P->ConsumeToken();
            Invoke->Params->Args.push_back(new ValueExpr(Loc, B));
            return ParseParameters(false, true);
        }
    }

    // Parse Params as Decl in Function Definition
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
    Function->Body = new StmtDecl(P->Tok.getLocation(), Function->Params->Vars);
    return P->ParseStmt(Function->Body, true);
}
