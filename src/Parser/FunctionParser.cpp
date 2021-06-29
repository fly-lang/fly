//
// Created by marco on 4/27/21.
//

#include <Parser/FunctionParser.h>
#include <vector>

using namespace fly;

FunctionParser::FunctionParser(Parser *P, const StringRef &FuncName, SourceLocation &FuncNameLoc)
                                    : P(P), FuncName(FuncName), FuncNameLoc(FuncNameLoc) {

}

bool FunctionParser::ParseCall(BlockStmt *CurrStmt, llvm::StringRef NameSpace) {
    Call = new FuncCall(FuncNameLoc, NameSpace, FuncName);
    return ParseArgs();
}

bool FunctionParser::ParseDefinition(TypeBase *TyDecl) {
    Function = new FuncDecl(P->AST, FuncNameLoc, TyDecl, FuncName);
    if (ParseParams()) {
        return ParseBody();
    }
    return false;
}

bool FunctionParser::ParseParams() {
    return ParseHeader(true, false);
}

bool FunctionParser::ParseArgs() {
    return ParseHeader(true, true);
}

bool FunctionParser::ParseHeader(bool isStart, bool isRef) {
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
        if (P->Tok.isAnyIdentifier()) { // FIXME to parse also functions
            VarRef *Var = P->ParseVarRef();
            VarRefExpr *RExpr = new VarRefExpr(P->Tok.getLocation(), Var);
            Call->addArg(RExpr);
            return ParseHeader(false, true);
        } else if (P->isValue()) {
            ValueExpr *ValExp = P->ParseValueExpr();
            Call->addArg(ValExp);
            return ParseHeader(false, true);
        }
    }

    // Parse Params as Decl in Function Definition
    FuncParam *Param = ParseParam();
    if (Param) {
        Function->Header->Params.push_back(Param);
        return ParseHeader();
    } else {
        P->Diag(P->Tok.getLocation(), diag::err_func_param);
        return false;
    }
}

FuncParam* FunctionParser::ParseParam() {
    // Var Constant
    bool Constant = false;
    P->ParseConstant(Constant);

    // Var Type
    TypeBase *TyDecl = P->ParseType();

    // Var Name
    const StringRef Name = P->Tok.getIdentifierInfo()->getName();
    const SourceLocation IdLoc = P->Tok.getLocation();
    P->ConsumeToken();
    FuncParam *Param = new FuncParam(IdLoc, TyDecl, Name);
    Param->Constant = Constant;

    // Parse assignment =
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();

        // Start Parsing
        if (P->isValue()) {
            GroupExpr *Group = new GroupExpr();
            ValueExpr *Val = P->ParseValueExpr();
            if (Val) {
                Group->Add(Val);
            }
            Param->Expression = Group;
        }
    }

    return Param;
}

bool FunctionParser::ParseBody() {
//    if (!Function->Params->Params.empty()) {
//        for (ParamDecl *Param : Function->Params->Params) {
//            Function->Body->Vars.insert(std::pair<StringRef, VarDeclStmt *>(Param->Name, Param));
//        } FIXME to remove if checking var existence directly from parameters
//    }
    if (P->Tok.is(tok::l_brace)) {
        P->ConsumeBrace();
        if (P->ParseAllInBraceStmt(Function->Body)) {
            return P->isBraceBalanced();
        }
    }
}
