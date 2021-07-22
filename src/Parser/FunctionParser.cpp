//
// Created by marco on 4/27/21.
//

#include <Parser/FunctionParser.h>
#include <vector>

using namespace fly;

FunctionParser::FunctionParser(Parser *P, const StringRef &FuncName, SourceLocation &FuncNameLoc)
                                    : P(P), FuncName(FuncName), FuncNameLoc(FuncNameLoc) {

}

bool FunctionParser::ParseCall(ASTBlock *Block, llvm::StringRef NameSpace) {
    Call = new ASTFuncCall(FuncNameLoc, NameSpace, FuncName);
    return ParseArgs(Block);
}

bool FunctionParser::ParseDecl(ASTType *TyDecl) {
    Function = new ASTFunc(P->AST, FuncNameLoc, TyDecl, FuncName);
    if (ParseParams()) {
        return ParseBody();
    }
    return false;
}

bool FunctionParser::ParseBody() {
    if (P->Tok.is(tok::l_brace)) {
        P->ConsumeBrace();
        if (P->ParseInnerBlock(Function->Body)) {
            return P->isBraceBalanced();
        }
    }

    return false;
}

bool FunctionParser::ParseParams() {
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

bool FunctionParser::ParseParam() {
    // Var Constant
    bool Constant = false;
    P->ParseConstant(Constant);

    // Var Type
    ASTType *TyDecl = P->ParseType();

    // Var Name
    const StringRef Name = P->Tok.getIdentifierInfo()->getName();
    const SourceLocation IdLoc = P->Tok.getLocation();
    P->ConsumeToken();
    ASTFuncParam *Param = new ASTFuncParam(IdLoc, TyDecl, Name);
    Param->Constant = Constant;

    // Parse assignment =
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();

        // Start Parsing
        if (P->isValue()) {
            ASTGroupExpr *Group = new ASTGroupExpr();
            ASTValueExpr *Val = P->ParseValueExpr();
            if (Val) {
                Group->Add(Val);
            }
            Param->Expression = Group;
        }
    }

    Function->Header->Params.push_back(Param);

    if (P->Tok.is(tok::comma)) {
        P->ConsumeToken();
        return ParseParam();
    }

    if (P->Tok.is(tok::r_paren)) {
        P->ConsumeParen();
        return true; // end
    }

    P->Diag(P->Tok.getLocation(), diag::err_func_param);
    return false;
}

bool FunctionParser::ParseArgs(ASTBlock *Block, bool isStart) {
    if (P->Tok.is(tok::l_paren)) { // parse start of function ()
        P->ConsumeParen(); // consume l_paren
    }

    if (P->Tok.is(tok::r_paren)) {
        P->ConsumeParen();
        return true; // end
    }

    return ParseArg(Block);
}

bool FunctionParser::ParseArg(ASTBlock *Block) {
    // Parse Args in a Function Call
    ASTExpr *E = P->ParseExpr(Block);

    // Type will be resolved into AST Finalize
    ASTType *Ty = nullptr;
    ASTFuncArg *Arg = new ASTFuncArg(E, Ty);
    Call->addArg(Arg);

    if (P->Tok.is(tok::comma)) {
        P->ConsumeToken();
        return ParseArg(Block);
    }

    if (P->Tok.is(tok::r_paren)) {
        P->ConsumeParen();
        return true; // end
    }

    P->Diag(P->Tok.getLocation(), diag::err_func_param);
    return false;
}