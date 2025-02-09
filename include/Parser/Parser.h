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
    class ASTArrayTypeRef;
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

    /// Constructor for the Parser class.
    Parser(const InputFile &Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags, ASTBuilder &Builder);

    /// Parse the main module.
    ASTModule *ParseModule();

    /// Parse the header of the module.
    ASTModule *ParseHeader();

    /// Check if the parsing was successful.
    bool isSuccess();

private:

    /// Parse names.
    llvm::SmallVector<llvm::StringRef, 4> ParseNames();

    /// Parse a namespace.
    ASTNameSpace *ParseNameSpace();

    /// Parse an import statement.
    ASTImport * ParseImport();

    /// Parse a definition.
    ASTBase *ParseDefinition();

    /// Parse multiple scopes.
    SmallVector<ASTScope *, 8> ParseScopes();

    /// Parse a global variable.
    ASTVar *ParseGlobalVar(SmallVector<ASTScope *, 8> &Scopes, ASTTypeRef *TypeRef);

    /// Parse a function.
    ASTFunction *ParseFunction(SmallVector<ASTScope *, 8> &Scopes, ASTTypeRef *TypeRef);

    /// Parse a class.
    ASTClass *ParseClass(SmallVector<ASTScope *, 8> &Scopes);

    /// Parse an enum.
    ASTEnum *ParseEnum(SmallVector<ASTScope *, 8> &Scopes);

    /// Parse a comment.
    ASTComment *ParseComment();

    /// Parse a block or statement.
    void ParseBlockOrStmt(ASTBlockStmt *Parent);

    /// Parse a block.
    void ParseBlock(ASTBlockStmt *Parent);

    /// Parse a statement.
    void ParseStmt(ASTBlockStmt *Parent);

    /// Parse the start of a parenthesis.
    bool ParseStartParen();

    /// Parse the end of a parenthesis.
    void ParseEndParen(bool HasParen);

    /// Parse an if statement.
    void ParseIfStmt(ASTBlockStmt *Parent);

    /// Parse a switch statement.
    void ParseSwitchStmt(ASTBlockStmt *Parent);

    /// Parse a while statement.
    void ParseWhileStmt(ASTBlockStmt *Parent);

    /// Parse a for statement.
    void ParseForStmt(ASTBlockStmt *Parent);

    /// Parse a handle statement.
    void ParseHandleStmt(ASTBlockStmt *Parent, ASTVarRef *Error);

    /// Parse a fail statement.
    void ParseFailStmt(ASTBlockStmt *Parent);

    /// Parse an array type reference.
    ASTArrayTypeRef *ParseArrayTypeRef(ASTTypeRef *);

    /// Parse a type reference.
    ASTTypeRef *ParseTypeRef();

    /// Parse a call.
    ASTCall *ParseCall(ASTIdentifier *&Identifier);

    /// Parse an identifier.
    ASTIdentifier *ParseIdentifier(ASTIdentifier *Parent = nullptr, bool istype = false);

    /// Parse a value.
    ASTValue *ParseValue();

    /// Parse a number value.
    ASTValue *ParseValueNumber(llvm::StringRef Value);

    /// Parse multiple values.
    ASTValue *ParseValues();

    /// Parse an expression.
    ASTExpr *ParseExpr();

    /// Check if the token is a built-in type.
    bool isBuiltinType(Token &Tok);

    /// Check if the token is an array type.
    bool isArrayType(Token &Tok);

    /// Check if the token is a value.
    bool isValue();

    /// Check if the token is a constant.
    bool isConst();

    /// Check if the token is the start of a block.
    bool isBlockStart();

    /// Check if the token is the end of a block.
    bool isBlockEnd();

    /// Consume the current token.
    SourceLocation ConsumeToken();

    /// Check if the token is a parenthesis.
    bool isTokenParen() const;

    /// Check if the token is a bracket.
    bool isTokenBracket() const;

    /// Check if the token is a brace.
    bool isTokenBrace() const;

    /// Check if the token is a string literal.
    bool isTokenStringLiteral() const;

    /// Check if the token is a special character.
    bool isTokenSpecial() const;

    /// Check if the token is a comment.
    bool isTokenComment() const;

    /// Check if the token is an assignment operator.
    bool isAssignOperator(const Token &Tok) const;

    /// Consume a parenthesis token.
    SourceLocation ConsumeParen();

    /// Consume a bracket token.
    SourceLocation ConsumeBracket();

    /// Consume a brace token.
    SourceLocation ConsumeBrace(unsigned short &BraceCount);

    /// Check if the braces are balanced.
    bool isBraceBalanced() const;

    /// Consume a string token.
    SourceLocation ConsumeStringToken();

    /// Get the literal string from the token.
    llvm::StringRef getLiteralString();

    /// Emit a diagnostic message.
    DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);

    /// Emit a diagnostic message for a token.
    DiagnosticBuilder Diag(const Token &Tok, unsigned DiagID);

    /// Emit a diagnostic message.
    DiagnosticBuilder Diag(unsigned DiagID);

    /// Emit a diagnostic message for an invalid identifier.
    void DiagInvalidId(SourceLocation Loc);
};

}  // end namespace fly

#endif