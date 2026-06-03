//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/Parser.h - main parser
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
    class ASTBuilderSwitchStmt;
    class ASTNode;
    class ASTModule;
    class SemaNameSpace;
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
    class ASTIdentifier;
    class ASTModifier;
    class ASTClass;
    class ASTEnum;
    class ASTComment;
    class ASTNameSpace;
    class ASTName;
    class ASTImport;
    class ASTVar;
    class ASTArrayType;
    class ASTType;
    class ASTTypeParam;

    /// ParseModule the main file known to the preprocessor, producing an
    /// abstract syntax tree.
    class Parser {

    friend class ParserFunction;
    friend class ParserClass;
    friend class ParserEnum;
    friend class ParserExpr;
    friend class ParserMethod;

    InputFile *Input;

    DiagnosticsEngine &Diags;

    SourceManager &SourceMgr;

    ASTBuilder &Builder;

    Lexer Lex;

    /// Tok - The current token we are peeking ahead.  All parsing methods assume
    /// that this is valid.
    Token Tok;

    ASTModule *Module;

    bool ContinueParse =  true;

    // PrevTokLocation - The location of the token we previously
    // consumed. This token is used for diagnostics where we expected to
    // see a token following another token (e.g., the ';' at the end of
    // a statement).
    SourceLocation PrevTokLocation;

    unsigned short ParenCount = 0, BracketCount = 0, BraceCount = 0;

    // Stable string storage for synthetic names generated during parsing (e.g. "__out_N").
    // Stored here so StringRefs into these strings outlive the ParseStmt stack frame.
    llvm::SmallVector<std::string, 8> SyntheticNames;

public:

    /// Constructor for the Parser class.
    Parser(InputFile *Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags, ASTBuilder &Builder);

    /// Parse the main module.
    ASTModule *ParseModule();

    /// Parse the header of the module.
    ASTModule *ParseHeader();

    /// Check if the parsing was successful.
    bool isSuccess();

private:

    /// Parse a namespace.
    ASTNameSpace *ParseNameSpace();

    /// Parse an import statement.
    ASTImport * ParseImport();

    llvm::SmallVector<ASTName *, 4> ParseNames();

    /// Parse a comment.
    ASTComment *ParseComment();

    /// Parse a definition.
    void ParseNode();

    /// Parse multiple Modifiers.
    SmallVector<ASTModifier *, 8> ParseModifiers();

    /// Parse a class or suite.
    ASTClass *ParseClass(SmallVector<ASTModifier *, 8> &Modifiers, bool SkipBodies = false);

    /// Parse an enum.
    ASTEnum *ParseEnum(SmallVector<ASTModifier *, 8> &Modifiers);

    /// Parse a function.
    ASTFunction *ParseFunction(SmallVector<ASTModifier *, 8> &Modifiers);

    /// Parse a block or statement.
    void ParseBlockOrStmt(ASTBlockStmt *Parent);

    /// Parse a block.
    void ParseBlock(ASTBlockStmt *Parent);

    /// Parse a statement.
    void ParseStmt(ASTBlockStmt *Parent);

    bool isType(std::optional<Token> &NexTok);

    bool isVarDecl(std::optional<Token> &NexTok);

    /// Check if is an assignment.
    bool isVarAssign(std::optional<Token> &NexTok) const;

    bool isVar(std::optional<Token> &NexTok);

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
    void ParseHandleStmt(ASTBlockStmt *Parent);

    /// Parse a fail statement.
    void ParseFailStmt(ASTBlockStmt *Parent);

    /// Parse an inline test block: "test { ... }"
    void ParseTestStmt(ASTBlockStmt *Parent);

    /// Parse a standalone "case" statement (suite test-method context).
    void ParseSuiteCase(ASTBlockStmt *Parent);

    /// Parse a type reference.
    ASTType *ParseType();

    /// Parse generic type arguments: <int, string>
    llvm::SmallVector<ASTType *, 4> ParseTypeArguments();

    /// Parse generic type parameter declarations: <T, U extends Foo>
    llvm::SmallVector<ASTTypeParam *, 4> ParseTypeParams();

    /// Parse an expression.
    ASTExpr *ParseExpr(ASTExpr *Left = nullptr);

    /// Parse  an identifier.
    ASTExpr *ParseIdentifier();

    /// Check if the token is a built-in type.
    bool isBuiltinType(Token &Tok);

    /// Check if the current identifier token is a named return type (not the function name).
    bool isNamedReturnType();

    /// Check if the token is an array type.
    bool isArrayType(Token &Tok);

    /// Check if the token is a value.
    bool isValue();

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

    bool isAnyOperator(const Token &Tok) const;

    /// Check if the token is an assignment operator.
    bool isAssignOperator(Token &Tok) const;

    /// Consume a parenthesis token.
    SourceLocation ConsumeParen();

    /// Consume a bracket token.
    SourceLocation ConsumeBracket();

    /// Consume a brace token.
    SourceLocation ConsumeBrace(unsigned short &BraceCount);

    /// Check if the braces are balanced.
    bool isBraceBalanced() const;

    /// Skip a brace-delimited block, tracking nesting depth.
    void SkipBraceBlock();

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