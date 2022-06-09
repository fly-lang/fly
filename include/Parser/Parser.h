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

#include <AST/ASTBlock.h>
#include <AST/ASTIfBlock.h>
#include <AST/ASTSwitchBlock.h>
#include <AST/ASTForBlock.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTStmt.h>
#include <AST/ASTVar.h>
#include <AST/ASTExpr.h>
#include <AST/ASTFunction.h>
#include "Frontend/InputFile.h"
#include "FunctionParser.h"
#include "ClassParser.h"
#include "Lex/Token.h"
#include "Lex/Lexer.h"
#include "AST/ASTNode.h"

namespace fly {

    class DiagnosticsEngine;
    class SemaBuilder;
    class Lexer;
    class ASTValue;
    class ASTSingleValue;
    class ASTArrayValue;

    /// Parse the main file known to the preprocessor, producing an
    /// abstract syntax tree.
    class Parser {

        friend class GlobalVarParser;
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

        std::string BlockComment;

    public:

        Parser(const InputFile &Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags, SemaBuilder &Builder);

        bool Parse();
        bool ParseHeader();

        ASTNode *getNode() const;

    private:

        // Parse NameSpace
        bool ParseNameSpace();

        // Parse Imports
        bool ParseImports();

        // Parse Top Definitions
        bool ParseTopDef();
        bool ParseTopScopes(VisibilityKind &Visibility, bool &isConst, bool isParsedVisibility = false,
                            bool isParsedConstant = false);
        bool ParseGlobalVar(VisibilityKind &Visibility, bool &Constant, ASTType *Type);
        bool ParseFunction(VisibilityKind &Visibility, bool Constant, ASTType *Type);
        bool ParseClass(VisibilityKind &Visibility, bool &Constant);

        // Parse Block Statement
        bool ParseBlock(ASTBlock *Block);
        bool ParseInnerBlock(ASTBlock *Block);
        bool ParseStmt(ASTBlock *Block);
        bool ParseStartParen();
        bool ParseEndParen(bool HasParen);
        bool ParseIfStmt(ASTBlock *Block);
        bool ParseSwitchStmt(ASTBlock *Block);
        bool ParseWhileStmt(ASTBlock *Block);
        bool ParseForStmt(ASTBlock *Block);
        bool ParseForCommaStmt(ASTBlock *Block);

        // Parse Identifiers
        ASTType *ParseType();
        ASTVarRef *ParseVarRef();
        bool ParseIdentifier(SourceLocation &Loc, llvm::StringRef &Name, llvm::StringRef &NameSpace);

        // Parse Function Calls
        ASTFunctionCall *ParseFunctionCall(ASTStmt *Stmt, SourceLocation &Loc, llvm::StringRef Name, llvm::StringRef NameSpace);
        bool ParseCallArgs(ASTFunctionCall *Call);
        bool ParseCallArg(ASTFunctionCall *Call);

        // Parse a Value
        ASTValue *ParseValue();
        bool ParseValues(ASTArrayValue &ArrayValues);

        // Parse Expressions
        ASTExpr *ParseExpr(ASTStmt *Stmt = nullptr);

        // Check Keywords
        bool isType(Optional<Token> &Tok1);
        bool isBuiltinType(Token &Tok);
        bool isIdentifier();
        bool isIdentifier(Optional<Token> &Tok1);
        bool isValue();
        bool isConst();

        // Parse Tokens
        SourceLocation ConsumeToken();
        bool isTokenParen() const;
        bool isTokenBracket() const;
        bool isTokenBrace() const;
        bool isTokenStringLiteral() const;
        bool isTokenSpecial() const;
        bool isTokenOperator() const;
        bool isTokenAssign() const;
        bool isTokenAssignOperator() const;
        bool isTokenAssignOperator(Optional<Token> OptTok) const;
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
        void ClearBlockComment();

        // Diagnostics
        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);
        DiagnosticBuilder Diag(const Token &Tok, unsigned DiagID);
        DiagnosticBuilder Diag(unsigned DiagID);
        void DiagInvalidId(SourceLocation Loc);
    };

}  // end namespace fly

#endif