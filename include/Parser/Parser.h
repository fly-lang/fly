//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/Parser.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_PARSE_PARSEAST_H
#define FLY_PARSE_PARSEAST_H

#include <AST/StmtDecl.h>
#include <AST/IfStmtDecl.h>
#include <AST/SwitchStmtDecl.h>
#include <AST/ForStmtDecl.h>
#include <AST/VarDecl.h>
#include <AST/Decl.h>

#include "GlobalVarParser.h"
#include "FunctionParser.h"
#include "ClassParser.h"
#include "Lex/Token.h"
#include "Lex/Lexer.h"
#include "AST/ASTNode.h"

namespace fly {

    class DiagnosticsEngine;
    class Lexer;

    /// ParseDefinition the main file known to the preprocessor, producing an
    /// abstract syntax tree.
    class Parser {

        friend class GlobalVarParser;
        friend class FunctionParser;

        DiagnosticsEngine &Diags;

        Lexer &Lex;

        ASTNode *AST;

        /// Tok - The current token we are peeking ahead.  All parsing methods assume
        /// that this is valid.
        Token Tok;

        // PrevTokLocation - The location of the token we previously
        // consumed. This token is used for diagnostics where we expected to
        // see a token following another token (e.g., the ';' at the end of
        // a statement).
        SourceLocation PrevTokLocation;

        unsigned short ParenCount = 0, BracketCount = 0, BraceCount = 0;

    public:

        Parser(Lexer &Lex, DiagnosticsEngine &Diags);

        bool Parse(ASTNode* Unit);

        ASTNode* getAST();

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);
        DiagnosticBuilder Diag(const Token &Tok, unsigned DiagID);
        DiagnosticBuilder Diag(unsigned DiagID) {
            return Diag(Tok, DiagID);
        }

    private:

        /// ConsumeToken - Consume the current 'peek token' and lex the next one.
        /// This does not work with special tokens: string literals,
        /// annotation tokens and balanced tokens must be handled using the specific
        /// consume methods.
        /// Returns the location of the consumed token.
        SourceLocation ConsumeToken() {
            assert(!isTokenSpecial() &&
                   "Should consume special tokens with Consume*Token");
            return ConsumeNext();
        }

        /// isTokenParen - Return true if the cur token is '(' or ')'.
        bool isTokenParen() const {
            return Tok.isOneOf(tok::l_paren, tok::r_paren);
        }

        /// isTokenBracket - Return true if the cur token is '[' or ']'.
        bool isTokenBracket() const {
            return Tok.isOneOf(tok::l_square, tok::r_square);
        }

        /// isTokenBrace - Return true if the cur token is '{' or '}'.
        bool isTokenBrace() const {
            return Tok.isOneOf(tok::l_brace, tok::r_brace);
        }

        /// isTokenConsumeParenStringLiteral - True if this token is a string-literal.
        bool isTokenStringLiteral() const {
            return tok::isStringLiteral(Tok.getKind());
        }

        /// isTokenSpecial - True if this token requires special consumption methods.
        bool isTokenSpecial() const {
            return isTokenStringLiteral() || isTokenParen() || isTokenBracket() ||
                   isTokenBrace();
        }

        /// ConsumeParen - This consume method keeps the paren count up-to-date.
        ///
        SourceLocation ConsumeParen() {
            assert(isTokenParen() && "wrong consume method");
            if (Tok.getKind() == tok::l_paren)
                ++ParenCount;
            else if (ParenCount) {
                //AngleBrackets.clear(*this);
                --ParenCount;       // Don't let unbalanced )'s drive the count negative.
            }

            return ConsumeNext();
        }

        /// ConsumeBracket - This consume method keeps the bracket count up-to-date.
        ///
        SourceLocation ConsumeBracket() {
            assert(isTokenBracket() && "wrong consume method");
            if (Tok.getKind() == tok::l_square)
                ++BracketCount;
            else if (BracketCount) {
                //AngleBrackets.clear(*this);
                --BracketCount;     // Don't let unbalanced ]'s drive the count negative.
            }

            return ConsumeNext();
        }

        /// ConsumeBrace - This consume method keeps the brace count up-to-date.
        ///
        SourceLocation ConsumeBrace() {
            assert(isTokenBrace() && "wrong consume method");
            if (Tok.getKind() == tok::l_brace)
                ++BraceCount;
            else if (BraceCount) {
                //AngleBrackets.clear(*this);
                --BraceCount;     // Don't let unbalanced }'s drive the count negative.
            }

            return ConsumeNext();
        }

        bool isBraceBalanced() const {
            return BraceCount == 0;
        }

        /// ConsumeStringToken - Consume the current 'peek token', lexing a new one
        /// and returning the token kind.  This method is specific to strings, as it
        /// handles string literal concatenation, as per C99 5.1.1.2, translation
        /// phase #6.
        SourceLocation ConsumeStringToken() {
            assert(isTokenStringLiteral() &&
                   "Should only consume string literals with this method");

            return ConsumeNext();
        }

        SourceLocation ConsumeNext() {
            PrevTokLocation = Tok.getLocation();
            Lex.Lex(Tok);
            return PrevTokLocation;
        }

        StringRef getLiteralString();

        bool ParseNameSpace();

        bool ParseImportDecl();

        bool ParseImportParenDecl();

        bool ParseImportAliasDecl(const SourceLocation &Loc, StringRef Name);

        bool ParseTopScopes(VisibilityKind &Visibility, bool &Constant);

        bool ParseScopes(bool &Constant);

        bool ParseTopDecl();

        bool ParseGlobalVarDecl(VisibilityKind &VisKind, bool &Constant, TypeBase *TyDecl,
                                IdentifierInfo *Id, SourceLocation &IdLoc);

        bool ParseClassDecl(VisibilityKind &VisKind, bool &Constant);

        bool ParseFunctionDecl(VisibilityKind &VisKind, bool Constant, TypeBase *TyDecl, IdentifierInfo *Id,
                               SourceLocation &IdLoc);
        bool ParseOneStmt(StmtDecl *CurrentStmt, GroupExpr *Group = NULL);
        bool ParseAllStmt(StmtDecl *CurrentStmt);
        bool ParseAllInBraceStmt(StmtDecl *CurrentStmt);
        bool ParseStartParen();
        bool ParseEndParen(bool hasParen);
        bool ParseIfStmt(StmtDecl *CurrentStmt);
        bool ParseSwitchStmt(StmtDecl *CurrentStmt);
        bool ParseForStmt(StmtDecl *CurrentStmt);
        bool ParseInitForStmt(StmtDecl *InitStmt, GroupExpr *Cond);
        bool ParseCondForStmt(GroupExpr* Cond);
        bool ParsePostForStmt(StmtDecl *PostStmt);

        FuncRefDecl *ParseFunctionRefDecl(IdentifierInfo *Id, SourceLocation &IdLoc);
        VarDecl* ParseVarDecl();
        VarDecl* ParseVarDecl(bool Constant, TypeBase *TyDecl);
        VarRef* ParseVarRef();
        ValueExpr* ParseValueExpr();
        GroupExpr* ParseExpr(GroupExpr *CurrGroup = NULL);
        VarRefDecl* ParseIncDec(SourceLocation &Loc, IdentifierInfo *Id);

        bool isVoidType();
        bool isBuiltinType();
        bool isValue();
        bool isOpAssign();
        bool isOpIncDec();
        bool isOperator();

        TypeBase *ParseType();
        TypeBase *ParseType(SourceLocation Loc, tok::TokenKind Kind);
        StringRef ParseBoolValue();
        OperatorExpr* ParseOperator();
        GroupExpr* ParseOpAssign(VarRef *Ref);
        IncDecExpr* ParseOpIncrement(bool post = false);

        void ParseInternalStmt();
    };

}  // end namespace fly

#endif