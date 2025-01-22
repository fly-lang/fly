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

#include "Token.h"
#include "Lexer.h"

namespace fly {

    class DiagnosticsEngine;
    class SemaBuilder;
    class SemaBuilderSwitchStmt;
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
    class ASTIfStmt;
    class ASTFailStmt;
    class ASTSwitchStmt;
    class ASTLoopStmt;
    class ASTExpr;
    class InputFile;
    class ASTHandleStmt;
    class ASTVarRef;
    class ASTTopDef;
    class ASTScope;
    class ASTClass;
    class ASTGlobalVar;
    class ASTEnum;
    class ASTComment;
    enum class ASTBinaryOperatorKind;

    /// ParseModule the main file known to the preprocessor, producing an
    /// abstract syntax tree.
    class Parser {

        friend class ParserFunction;
        friend class ParserClass;
        friend class ParserEnum;
        friend class ParserExpr;
        friend class ParserMethod;

        const InputFile &Input;

        DiagnosticsEngine &Diags;

        SourceManager &SourceMgr;

        SemaBuilder &Builder;

        Lexer Lex;

        /// Tok - The current token we are peeking ahead.  All parsing methods assume
        /// that this is valid.
        Token Tok;

        ASTModule *Module;

        // PrevTokLocation - The location of the token we previously
        // consumed. This token is used for diagnostics where we expected to
        // see a token following another token (e.g., the ';' at the end of
        // a statement).
        SourceLocation PrevTokLocation;

        unsigned short ParenCount = 0, BracketCount = 0, BraceCount = 0;

    public:

        Parser(const InputFile &Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags, SemaBuilder &Builder);

        ASTModule *ParseModule();
        ASTModule *ParseHeader();

        bool isSuccess();

    private:

        void Parse();
        ASTBase *ParseTopDef(ASTComment *Comment, SmallVector<ASTScope *, 8>& Scopes);
        SmallVector<ASTScope *, 8> ParseScopes();
        ASTGlobalVar *ParseGlobalVarDef(ASTComment *Comment, SmallVector<ASTScope *, 8> &Scopes, ASTType *Type);
        ASTFunction *ParseFunctionDef(ASTComment *Comment, SmallVector<ASTScope *, 8> &Scopes, ASTType *Type);
        ASTClass *ParseClassDef(ASTComment *Comment, SmallVector<ASTScope *, 8> &Scopes);
        ASTEnum *ParseEnumDef(ASTComment *Comment, SmallVector<ASTScope *, 8> &Scopes);
        ASTComment *ParseComments();
        void SkipComments();

        // Parse Stmt
        bool ParseBlockOrStmt(ASTBlockStmt *Parent);
        bool ParseBlock(ASTBlockStmt *Parent);
        bool ParseStmt(ASTBlockStmt *Parent);
        bool ParseStartParen(); // FIXME remove?
        bool ParseEndParen(bool HasParen); // FIXME remove?
        bool ParseIfStmt(ASTBlockStmt *Parent);
        bool ParseSwitchStmt(ASTBlockStmt *Parent);
        bool ParseSwitchCases(SemaBuilderSwitchStmt *SwitchBuilder);
        bool ParseWhileStmt(ASTBlockStmt *Parent);
        bool ParseForStmt(ASTBlockStmt *Parent);
        bool ParseHandleStmt(ASTBlockStmt *Parent, ASTVarRef *Error);
        bool ParseFailStmt(ASTBlockStmt *Parent);

        // Parse Identifiers
        ASTType *ParseBuiltinType();
        ASTArrayType *ParseArrayType(ASTType *);
        ASTType *ParseType();
        ASTCall *ParseCallIdentifier(ASTIdentifier *&Identifier);
        ASTIdentifier *ParseIdentifier(ASTIdentifier *Parent = nullptr);

        // Parse a Value
        ASTValue *ParseValue();
        ASTValue *ParseValueNumber(llvm::StringRef Input);
        ASTValue *ParseValues();

        // Parse Expressions
        ASTExpr *ParseExpr();

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
        bool isTokenComment() const;
        bool isAssignOperator(const Token &Tok) const;
        SourceLocation ConsumeParen();
        SourceLocation ConsumeBracket();
        SourceLocation ConsumeBrace(unsigned short &BraceCount);
        bool isBraceBalanced() const;
        SourceLocation ConsumeStringToken();
        llvm::StringRef getLiteralString();

        // Diagnostics
        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);
        DiagnosticBuilder Diag(const Token &Tok, unsigned DiagID);
        DiagnosticBuilder Diag(unsigned DiagID);
        void DiagInvalidId(SourceLocation Loc);
    };

}  // end namespace fly

#endif