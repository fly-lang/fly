//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/Parser.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_PARSER_H
#define FLY_PARSER_H

#include "Lex/Token.h"
#include "Lex/Lexer.h"

namespace fly {

    class DiagnosticsEngine;
    class SemaBuilder;
    class ASTNode;
    class ASTValue;
    class ASTIdentifier;
    class ASTArrayValue;
    class ASTArrayType;
    class ASTClassType;
    class ASTTopScopes;
    class ASTType;
    class ASTBlock;
    class ASTCall;
    class ASTStmt;
    class ASTExpr;
    class ASTVarRef;
    class InputFile;
    enum class ASTBinaryOperatorKind;

    /// Parse the main file known to the preprocessor, producing an
    /// abstract syntax tree.
    class Parser {

        friend class FunctionParser;
        friend class ClassParser;
        friend class ExprParser;

        const InputFile &Input;

        DiagnosticsEngine &Diags;

        SourceManager &SourceMgr;

        SemaBuilder &Builder;

        Lexer Lex;

        /// Tok - The current token we are peeking ahead.  All parsing methods assume
        /// that this is valid.
        Token Tok;

        ASTNode *Node;

        std::string NameSpace;

        // PrevTokLocation - The location of the token we previously
        // consumed. This token is used for diagnostics where we expected to
        // see a token following another token (e.g., the ';' at the end of
        // a statement).
        SourceLocation PrevTokLocation;

        unsigned short ParenCount = 0, BracketCount = 0, BraceCount = 0;

        llvm::StringRef BlockComment;

    public:

        Parser(const InputFile &Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags, SemaBuilder &Builder);

        ASTNode *Parse();
        ASTNode *ParseHeader();

    private:

        // Parse NameSpace
        bool ParseNameSpace();

        // Parse Imports
        bool ParseImports();

        // Parse Top Definitions
        bool ParseTopDef();
        ASTTopScopes *ParseTopScopes();
        bool ParseGlobalVarDef(ASTTopScopes *Scopes, ASTType *Type);
        bool ParseFunctionDef(ASTTopScopes *Scopes, ASTType *Type);
        bool ParseClassDef(ASTTopScopes *Scopes);

        // Parse Block Statement
        bool ParseBlock(ASTBlock *Block);
        bool ParseStmt(ASTBlock *Block);
        bool ParseStartParen();
        bool ParseEndParen(bool HasParen);
        bool ParseIfStmt(ASTBlock *Block);
        bool ParseSwitchStmt(ASTBlock *Block);
        bool ParseWhileStmt(ASTBlock *Block);
        bool ParseForStmt(ASTBlock *Block);
        bool ParseForCommaStmt(ASTBlock *Block);

        // Parse Identifiers
        ASTType *ParseBuiltinType();
        ASTArrayType *ParseArrayType(ASTType *);
        ASTClassType *ParseClassType(ASTClassType *Parent = nullptr);
        ASTType *ParseType();
        ASTCall *ParseCall(ASTStmt *Stmt, ASTIdentifier *Identifier);
        bool ParseCallArg(ASTStmt *Stmt, ASTCall *Call);
        ASTIdentifier *ParseIdentifier();

        // Parse a Value
        ASTValue *ParseValue();
        ASTValue *ParseValueNumber(std::string &Str);
        bool ParseValues(ASTArrayValue &ArrayValues);

        // Parse Expressions
        ASTExpr *ParseExpr(ASTStmt *Stmt = nullptr, ASTIdentifier *Identifier = nullptr);

        // Check Keywords
        bool isBuiltinType(Token &Tok);
        bool isArrayType(Token &Tok);
        bool isClassType(Token &Tok);
        bool isValue();
        bool isConst();
        bool isBlockStart();
        bool isBlockEnd();

        // Parse Tokens
        SourceLocation ConsumeToken();
        bool isTokenParen() const;
        bool isTokenBracket() const;
        bool isTokenBrace() const;
        bool isTokenStringLiteral() const;
        bool isTokenSpecial() const;
        bool isTokenAssignOperator() const;
        bool isNewOperator(Token &Tok);
        bool isUnaryPreOperator(Token &Tok);
        bool isUnaryPostOperator();
        bool isBinaryOperator();
        bool isTernaryOperator();
        SourceLocation ConsumeParen();
        SourceLocation ConsumeBracket();
        SourceLocation ConsumeBrace();
        bool isBraceBalanced() const;
        SourceLocation ConsumeStringToken();
        SourceLocation ConsumeNext();
        llvm::StringRef getLiteralString();

        // Diagnostics
        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);
        DiagnosticBuilder Diag(const Token &Tok, unsigned DiagID);
        DiagnosticBuilder Diag(unsigned DiagID);
        void DiagInvalidId(SourceLocation Loc);
    };

}  // end namespace fly

#endif