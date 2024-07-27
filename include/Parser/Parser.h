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
    class ASTBase;
    class ASTModule;
    class ASTNameSpace;
    class ASTValue;
    class ASTIdentifier;
    class ASTFunction;
    class ASTArrayValue;
    class ASTArrayType;
    class ASTClassType;
    class ASTType;
    class ASTBlockStmt;
    class ASTCall;
    class ASTStmt;
    class ASTSwitchStmt;
    class ASTExpr;
    class InputFile;
    class ASTHandleStmt;
    class ASTVarRef;
    class ASTTopDef;
    class ASTScope;
    class ASTClass;
    class ASTGlobalVar;
    class ASTEnum;
    enum class ASTBinaryOperatorKind;

    /// ParseModule the main file known to the preprocessor, producing an
    /// abstract syntax tree.
    class Parser {

        friend class FunctionParser;
        friend class ClassParser;
        friend class EnumParser;
        friend class ExprParser;

        const InputFile &Input;

        DiagnosticsEngine &Diags;

        SourceManager &SourceMgr;

        SemaBuilder &Builder;

        Lexer Lex;

        /// Tok - The current token we are peeking ahead.  All parsing methods assume
        /// that this is valid.
        Token Tok;

        ASTModule *Module;

        bool ContinueParsing = true;

        // PrevTokLocation - The location of the token we previously
        // consumed. This token is used for diagnostics where we expected to
        // see a token following another token (e.g., the ';' at the end of
        // a statement).
        SourceLocation PrevTokLocation;

        unsigned short ParenCount = 0, BracketCount = 0, BraceCount = 0;

        llvm::StringRef BlockComment;

    public:

        Parser(const InputFile &Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags, SemaBuilder &Builder);

        ASTModule *ParseModule();
        ASTModule *ParseHeader();

        bool isSuccess();

    private:

        ASTBase *Parse();
        ASTBase *ParseTopDef(SmallVector<ASTScope *, 8>& Scopes);
        SmallVector<ASTScope *, 8> ParseScopes();
        ASTGlobalVar *ParseGlobalVarDef(SmallVector<ASTScope *, 8> &Scopes, ASTType *Type);
        ASTFunction *ParseFunctionDef(SmallVector<ASTScope *, 8> &Scopes, ASTType *Type);
        ASTClass *ParseClassDef(SmallVector<ASTScope *, 8> &Scopes);
        ASTEnum *ParseEnumDef(SmallVector<ASTScope *, 8> &Scopes);

        // Parse Block Statement
        bool ParseBlock(ASTBlockStmt *Parent);
        bool ParseStmt(ASTBlockStmt *Parent, bool StopParse = false);
        bool ParseStartParen();
        bool ParseEndParen(bool HasParen);
        bool ParseIfStmt(ASTBlockStmt *Parent);
        bool ParseSwitchStmt(ASTBlockStmt *Parent);
        bool ParseSwitchCases(ASTSwitchStmt *SwitchStmt);
        bool ParseWhileStmt(ASTBlockStmt *Parent);
        bool ParseForStmt(ASTBlockStmt *Parent);
        bool ParseHandleStmt(ASTBlockStmt *Parent, ASTVarRef *Error);
        bool ParseFailStmt(ASTBlockStmt *Parent);

        // Parse Identifiers
        bool ParseBuiltinType(ASTType *&);
        bool ParseArrayType(ASTType *&);
        bool ParseType(ASTType *&);
        bool ParseCall(ASTIdentifier *&Identifier);
        bool ParseCallArg(ASTCall *Call);
        ASTIdentifier *ParseIdentifier(ASTIdentifier *Parent = nullptr);

        // Parse a Value
        ASTValue *ParseValue();
        ASTValue *ParseValueNumber(std::string &Str);
        ASTValue *ParseValues();

        // Parse Expressions
        ASTExpr *ParseExpr(ASTIdentifier *Identifier = nullptr);

        // Check Keywords
        bool isBuiltinType(Token &Tok);
        bool isArrayType(Token &Tok);
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
        SourceLocation ConsumeBrace(unsigned short &BraceCount);
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