//
// Created by marco on 4/27/21.
//

#include <Parser/FunctionParser.h>

using namespace fly;

FunctionParser::FunctionParser(Parser *P, const StringRef &FuncName, SourceLocation &FuncNameLoc)
                                    : P(P), FuncName(FuncName), FuncNameLoc(FuncNameLoc) {

}

bool FunctionParser::ParseRefDecl(BlockStmt *CurrentStmt) {
    Call = new FuncCallStmt(FuncNameLoc, CurrentStmt, FuncName);
    return ParseParameters(true, true);
}

bool FunctionParser::ParseDefinition(TypeBase *TyDecl) {
    Function = new FuncDecl(FuncNameLoc, TyDecl, FuncName);
    Function->Body = new BlockStmt(P->Tok.getLocation(), Function, NULL);
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
            Call->Params->Args.push_back(RExpr);
            return ParseParameters(false, true);
        } else if (P->isValue()) {
            ValueExpr *ValExp = P->ParseValueExpr();
            Call->Params->Args.push_back(ValExp);
            return ParseParameters(false, true);
        }
    }

    // Parse Params as Decl in Function Definition
    VarDeclStmt *Var = P->ParseVarDecl(Function->Body);
    if (Var) {
        Function->Params->Vars.push_back(Var);
        return ParseParameters();
    } else {
        P->Diag(P->Tok.getLocation(), diag::err_func_param);
        return false;
    }
}

bool FunctionParser::ParseBody() {
    if (!Function->Params->Vars.empty()) {
        for (VarDeclStmt *Var : Function->Params->Vars) {
            Function->Body->Vars.insert(std::pair<StringRef, VarDeclStmt *>(Var->Name, Var));
        }
    }
    if (P->Tok.is(tok::l_brace)) {
        P->ConsumeBrace();
        if (P->ParseAllInBraceStmt(Function->Body)) {
            return P->isBraceBalanced();
        }
    }
}
