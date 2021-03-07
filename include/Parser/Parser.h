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

#include "Lex/Token.h"
#include "Lex/Lexer.h"
#include "AST/ASTContext.h"
#include "ParseDiagnostic.h"

namespace fly {
    class DiagnosticsEngine;

    class Lexer;

    /// Parse the main file known to the preprocessor, producing an
    /// abstract syntax tree.
    class Parser {

        Lexer &Lex;

        DiagnosticsEngine &Diags;

        /// Tok - The current token we are peeking ahead.  All parsing methods assume
        /// that this is valid.
        Token Tok;

        // PrevTokLocation - The location of the token we previously
        // consumed. This token is used for diagnostics where we expected to
        // see a token following another token (e.g., the ';' at the end of
        // a statement).
        SourceLocation PrevTokLocation;

        ASTContext *Context = nullptr;

    public:

        Parser(const std::string &fileName, Lexer &L, DiagnosticsEngine &diags);

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);
        DiagnosticBuilder Diag(const Token &Tok, unsigned DiagID);
        DiagnosticBuilder Diag(unsigned DiagID) {
            return Diag(Tok, DiagID);
        }

        /// ConsumeToken - Consume the current 'peek token' and lex the next one.
        /// This does not work with special tokens: string literals, code completion,
        /// annotation tokens and balanced tokens must be handled using the specific
        /// consume methods.
        /// Returns the location of the consumed token.
        SourceLocation ConsumeToken() {
            assert(!isTokenSpecial() &&
                   "Should consume special tokens with Consume*Token");
            PrevTokLocation = Tok.getLocation();
            Lex.Lex(Tok);
            return PrevTokLocation;
        }

        ASTContext &getASTContext();

    private:

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

        /// isTokenStringLiteral - True if this token is a string-literal.
        bool isTokenStringLiteral() const {
            return tok::isStringLiteral(Tok.getKind());
        }

        /// isTokenSpecial - True if this token requires special consumption methods.
        bool isTokenSpecial() const {
            return isTokenStringLiteral() || isTokenParen() || isTokenBracket() ||
                   isTokenBrace();
        }

        bool parsePackageDecl(const string& fileName);
    };


    }  // end namespace clang

#endif