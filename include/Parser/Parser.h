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

#include <AST/ASTBlock.h>
#include <AST/ASTIfBlock.h>
#include <AST/ASTSwitchBlock.h>
#include <AST/ASTForBlock.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTStmt.h>
#include <AST/ASTVar.h>
#include <AST/ASTExpr.h>
#include <AST/ASTFunc.h>
#include "Frontend/InputFile.h"
#include "GlobalVarParser.h"
#include "FunctionParser.h"
#include "ClassParser.h"
#include "Lex/Token.h"
#include "Lex/Lexer.h"
#include "AST/ASTNode.h"

namespace fly {

    class DiagnosticsEngine;
    class Lexer;

    /// Parse the main file known to the preprocessor, producing an
    /// abstract syntax tree.
    class Parser {

        friend class GlobalVarParser;
        friend class FunctionParser;

        const InputFile &Input;

        DiagnosticsEngine &Diags;

        Lexer Lex;

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

        Parser(const InputFile &Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags);

        bool Parse(ASTNode* Unit);
        bool ParseHeader(ASTNode *Node);

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);
        DiagnosticBuilder Diag(const Token &Tok, unsigned DiagID);
        DiagnosticBuilder Diag(unsigned DiagID);
        void DiagInvalidId(SourceLocation Loc);

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

        llvm::StringRef getLiteralString();

        std::string ParseNameSpace();

        bool ParseImports();

        bool ParseTopDecl();

        bool ParseTopScopes(VisibilityKind &Visibility, bool &isConst,
                            bool isParsedVisibility = false, bool isParsedConstant = false);

        bool ParseConst(bool &Constant);

        /**
         * Parse Global Var declaration
         * @param VisKind
         * @param Constant
         * @param Type
         * @param Id
         * @param IdLoc
         * @return
         */
        bool ParseGlobalVarDecl(VisibilityKind &VisKind, bool &Constant, ASTType *Type,
                                llvm::StringRef &Name, SourceLocation &NameLoc);

        /**
         * Parse Class declaration
         * @param VisKind
         * @param Constant
         * @return
         */
        bool ParseClassDecl(VisibilityKind &VisKind, bool &Constant);

        /**
         * Parse Function declaration
         * @param VisKind
         * @param Constant
         * @param Type
         * @param Id
         * @param IdLoc
         * @return
         */
        bool ParseFunction(VisibilityKind &VisKind, bool Constant, ASTType *Type, llvm::StringRef &Name,
                           SourceLocation &NameLoc);

        /**
         * Parse Type
         * @return
         */
        bool ParseType(ASTType *&Type, bool OnlyBuiltin = false);

        // Parse Block Statement

        bool ParseBlock(ASTBlock *Block);
        bool ParseInnerBlock(ASTBlock *Block);

        /**
         * Parse a Statement
         * @param Block
         * @return
         */
        bool ParseStmt(ASTBlock *Block);
        bool ParseIdentifier(llvm::StringRef &Name, llvm::StringRef &NameSpace, SourceLocation &Loc);

        // Parse Block Structures

        bool ParseStartParen();
        bool ParseEndParen(bool HasParen);
        bool ParseIfStmt(ASTBlock *Block);
        bool ParseSwitchStmt(ASTBlock *Block);
        bool ParseWhileStmt(ASTBlock *Block);
        bool ParseForStmt(ASTBlock *Block);
        bool ParseForCommaStmt(ASTBlock *Block);

        // Parse Calls, Vars

        ASTFuncCall *ParseFunctionCall(ASTBlock *Block, llvm::StringRef Name, llvm::StringRef NameSpace, SourceLocation &Loc, bool &Success);
        ASTLocalVar* ParseLocalVar(ASTBlock *Block, bool Constant, ASTType *Type, bool &Success);
        ASTVarRef* ParseVarRef(bool &Success);

        // Parse Expressions

        ASTExpr* ParseAssignmentExpr(ASTBlock *Block, ASTVarRef *VarRef, bool &Success);
        ASTExpr* ParseExpr(ASTBlock *Block, bool &Success);
        ASTExpr* ParseUnaryPreExpr(ASTBlock *Block, ASTVarRef *VarRef, bool &Success);
        ASTExpr* ParseUnaryPostExpr(ASTBlock *Block, bool &Success);
        ASTExpr* ParseBinaryExpr(ASTBlock *Block, ASTExpr *First, bool &Success);
        ASTExpr* ParseTernaryExpr(ASTBlock *Block, ASTExpr *First, bool &Success);
        ASTValueExpr* ParseValueExpr(bool &Success);

        // Check Keywords

        bool isBuiltinType();
        bool isValue();
        bool isUnaryPreOperator();
        bool isUnaryPostOperator();
        bool isBinaryOperator();
        bool isTernaryOperator();
    };

}  // end namespace fly

#endif