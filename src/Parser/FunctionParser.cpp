//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/FunctionParser.cpp - Function Declaration and Call Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <Parser/FunctionParser.h>
#include <vector>

using namespace fly;

/**
 * FunctionParser Constructor
 * @param P
 * @param FuncName
 * @param FuncNameLoc
 */
FunctionParser::FunctionParser(Parser *P) : P(P) {

}

/**
 * Parse Function Declaration
 * @param Type
 * @return true on Success or false on Error
 */
bool FunctionParser::ParseFunction(ASTType *Type) {
    assert(P->Tok.isAnyIdentifier() && "Tok must be an Identifier");

    IdentifierInfo *Id = P->Tok.getIdentifierInfo();
    llvm::StringRef Name = Id->getName();
    SourceLocation Loc = P->Tok.getLocation();

    AST = new ASTFunc(P->AST, Loc, Type, Name.str());

    // Add Comment to AST
    if (!P->BlockComment.empty()) {
        AST->setComment(P->BlockComment);
        P->ClearBlockComment(); // Clear for next use
    }

    P->ConsumeToken();
    return ParseFunctionParams();
}

/**
 * Parse Function Body
 * @return true on Success or false on Error
 */
bool FunctionParser::ParseFunctionBody() {
    if (P->Tok.is(tok::l_brace)) {
        P->ConsumeBrace();
        if (P->ParseInnerBlock(AST->Body)) {
            if (P->isBraceBalanced()) {
                // If next Top declaration do not have a block comment it could be taken the last from function body
                P->ClearBlockComment();
                return true;
            }
        }
    }

    return false;
}

/**
 * Parse Parameters
 * @return true on Success or false on Error
 */
bool FunctionParser::ParseFunctionParams() {
    if (P->Tok.is(tok::l_paren)) { // parse start of function ()
        P->ConsumeParen(); // consume l_paren
    }

    if (P->Tok.is(tok::r_paren)) {
        P->ConsumeParen();
        return true; // end
    }

    // Parse Params as Decl in Function Definition
    return ParseFunctionParam();
}

/**
 * Parse a single Function Param
 * @return true on Success or false on Error
 */
bool FunctionParser::ParseFunctionParam() {
    bool Success = true;

    // Var Constant
    bool Constant = false;
    P->ParseConst(Constant);

    // Var Type
    ASTType *Type = nullptr;
    if (P->ParseType(Type)) {

        // Var Name
        const StringRef Name = P->Tok.getIdentifierInfo()->getName();
        const SourceLocation IdLoc = P->Tok.getLocation();
        P->ConsumeToken();
        ASTFuncParam *Param = new ASTFuncParam(IdLoc, Type, Name.str());
        Param->Constant = Constant;

        // Parse assignment =
        if (P->Tok.is(tok::equal)) {
            P->ConsumeToken();

            // Start Parsing
            if (P->isValue()) {
                ASTValue *Val = P->ParseValue();
                if (Val != nullptr) {
                    Param->setExpr(new ASTValueExpr(Val));
                }
            }
        }

        if (Success) {
            AST->Header->Params.push_back(Param);

            if (P->Tok.is(tok::comma)) {
                P->ConsumeToken();
                return ParseFunctionParam();
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
 * Parse a Function Call
 * @param Block
 * @param NameSpace
 * @return true on Success or false on Error
 */
bool FunctionParser::ParseCall(ASTBlock *Block, SourceLocation &Loc, llvm::StringRef Name, llvm::StringRef NameSpace) {
    Call = new ASTFuncCall(Loc, NameSpace.str(), Name.str());
    return ParseCallArgs(Block);
}

/**
 * Parse Call Arguments
 * @param Block
 * @return true on Success or false on Error
 */
bool FunctionParser::ParseCallArgs(ASTBlock *Block) {
    if (P->Tok.is(tok::l_paren)) { // parse start of function ()
        P->ConsumeParen(); // consume l_paren
    }

    if (P->Tok.is(tok::r_paren)) {
        P->ConsumeParen();
        return true; // end
    }

    return ParseCallArg(Block);
}

/**
 * Parse a single Call Argument
 * @param Block
 * @return true on Success or false on Error
 */
bool FunctionParser::ParseCallArg(ASTBlock *Block) {

    // Parse Args in a Function Call
    ASTExpr *Expr = P->ParseExpr(Block);

    if (Expr != nullptr) {
        // Type will be resolved into AST Finalize
        ASTType *Ty = nullptr;
        ASTCallArg *Arg = new ASTCallArg(Expr, Ty);
        Call->addArg(Arg);

        if (P->Tok.is(tok::comma)) {
            P->ConsumeToken();
            return ParseCallArg(Block);
        }

        if (P->Tok.is(tok::r_paren)) {
            P->ConsumeParen();
            return true; // end
        }
    }

    P->Diag(P->Tok.getLocation(), diag::err_func_param);
    return false;
}