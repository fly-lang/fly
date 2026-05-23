//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/ParserExpr.h - expression parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_PARSEREXPR_H
#define FLY_PARSEREXPR_H

#include "Parser.h"
#include "AST/ASTExpr.h"

namespace fly {

	class ASTBinary;
	class ASTTernary;
	enum class ASTCallKind;

	enum class Precedence {
		LOWEST,         // No operators
		ASSIGNMENT,     // =, +=, -=, etc.
		TERNARY,        // ?
		LOGICAL,        // ||, &&
		RELATIONAL,     // ==, !=, <, >, <=, >=
		ADDITIVE,       // +, -
		MULTIPLICATIVE, // *, /
		UNARY,          // -a, !a, ++a, --a, a++, a--
		PRIMARY         // Literals, Identifiers, Calls
	};

    class ParserExpr {

        friend class ASTExpr;

        Parser *P;

        ASTExpr *Left;

    public:

        ParserExpr(Parser *P, ASTExpr *Expr = nullptr);

        ASTExpr *Parse(ASTExpr *Left = nullptr);

        /// Parse an identifier or a call.
        ASTExpr *ParseIdentifierOrCall(ASTExpr *Parent = nullptr);

        /// Parse a value.
        ASTValue *ParseValue();

    private:
        ASTExpr *ParsePrimary();

        ASTBinary *ParseBinaryExpr(ASTExpr *LeftExpr, Token OpToken, Precedence Precedence);

        ASTTernary *ParseTernaryExpr(ASTExpr *ConditionExpr);

        ASTExpr *ParseNewExpr();

        /// Parse a call.
        ASTCall *ParseCall(const SourceLocation &Loc, llvm::StringRef Name, ASTCallKind CallKind, ASTExpr *Parent = nullptr);

        /// Parse multiple values.
        ASTValue *ParseValues();

        bool isNewOperator(Token &Tok);

        bool isUnaryPreOperator(Token &Tok);

        bool isUnaryPostOperator();

        bool isBinaryOperator();

        bool isTernaryOperator();

    };

}

#endif //FLY_PARSEREXPR_H
