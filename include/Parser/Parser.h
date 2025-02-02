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
    class ASTBuilder;
    class SemaBuilderSwitchStmt;
    class ASTBase;
    class ASTModule;
    class SymNameSpace;
    class ASTValue;
    class ASTIdentifier;
    class ASTFunction;
    class ASTArrayValue;
    class ASTArrayType;
    class ASTClassType;
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
    class ASTScope;
    class ASTClass;
    class ASTEnum;
    class ASTComment;
    class ASTNameSpace;
    class ASTImport;
    class ASTVar;
class ASTTypeRef;

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

        ASTBuilder &Builder;

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

        Parser(const InputFile &Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags, ASTBuilder &Builder);

        ASTModule *ParseModule();

        ASTModule *ParseHeader();

        bool isSuccess();

    private:

        ASTNameSpace *ParseNameSpace();

        ASTImport * ParseImport();

        ASTBase *ParseDefinition();

        SmallVector<ASTScope *, 8> ParseScopes();

        ASTVar *ParseGlobalVar(SmallVector<ASTScope *, 8> &Scopes, ASTType *Type);

        ASTFunction *ParseFunction(SmallVector<ASTScope *, 8> &Scopes, ASTType *Type);

        ASTClass *ParseClass(SmallVector<ASTScope *, 8> &Scopes);

        ASTEnum *ParseEnum(SmallVector<ASTScope *, 8> &Scopes);

        ASTComment *ParseComment();

        // Parse Block or Stmt
        void ParseBlockOrStmt(ASTBlockStmt *Parent);
        void ParseBlock(ASTBlockStmt *Parent);
        void ParseStmt(ASTBlockStmt *Parent);
        bool ParseStartParen(); // FIXME remove?
        void ParseEndParen(bool HasParen); // FIXME remove?
        void ParseIfStmt(ASTBlockStmt *Parent);
        void ParseSwitchStmt(ASTBlockStmt *Parent);
        void ParseWhileStmt(ASTBlockStmt *Parent);
        void ParseForStmt(ASTBlockStmt *Parent);
        void ParseHandleStmt(ASTBlockStmt *Parent, ASTVarRef *Error);
        void ParseFailStmt(ASTBlockStmt *Parent);

        // Parse Identifiers
        ASTArrayType *ParseArrayType(ASTTypeRef *);
        ASTTypeRef *ParseTypeRef();
        ASTCall *ParseCallIdentifier(ASTIdentifier *&Identifier);
        ASTIdentifier *ParseIdentifier(ASTIdentifier *Parent = nullptr, bool istype = false);

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