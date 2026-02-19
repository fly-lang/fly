//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/ClassParser.cpp - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "Parser/ParserFunction.h"
#include "Parser/ParserClass.h"
#include "AST/ASTClass.h"
#include "AST/ASTBuilder.h"
#include "Basic/Debug.h"

#include <AST/ASTMethod.h>

using namespace fly;

/**
 * ClassParser Constructor
 * @param P
 * @param Visibility
 * @param Constant
 */
ParserClass::ParserClass(Parser *P, SmallVector<ASTModifier *, 8> &Modifiers) : P(P) {
    FLY_DEBUG_START("ClassParser", "ClassParser");

    ASTClassKind ClassKind;
    if (P->Tok.is(tok::kw_struct)) {
        ClassKind = ASTClassKind::STRUCT;
    } else if (P->Tok.is(tok::kw_class)) {
        ClassKind = ASTClassKind::CLASS;
    } else if (P->Tok.is(tok::kw_interface)) {
        ClassKind = ASTClassKind::INTERFACE;
    } else {
        assert("No ClassKind defined");
    }
    P->ConsumeToken();

    // Parse class name
    llvm::StringRef ClassName = P->Tok.getIdentifierInfo()->getName();
    const SourceLocation ClassLoc = P->Tok.getLocation();
    P->ConsumeToken();

    // Parse classes after colon
    // class Example : SuperClass, Interface, Struct { ... }
    llvm::SmallVector<ASTType *, 4> Bases;
    if (P->Tok.is(tok::colon)) {
        P->ConsumeToken();
        while (P->Tok.isAnyIdentifier()) {
            ASTType *ClassTypeRef = P->ParseType();
            Bases.push_back(ClassTypeRef);

            // Consume comma if present
            if (P->Tok.is(tok::comma)) {
                P->ConsumeToken();
            } else {
                // No comma, we're done with base classes
                break;
            }
        }
    }

    // Parse block in the braces
    if (P->isBlockStart()) {
        P->ConsumeBrace(BraceCount);

        Class = ASTBuilder::CreateClass(P->Module, ClassLoc, ClassKind, ClassName, Modifiers, Bases);
        bool Continue;
        do {

            // End of the Class
            if (P->isBlockEnd() ) {
                P->ConsumeBrace(BraceCount);
                break;
            }

            // Error: Class block not correctly closed
            if (P->Tok.is(tok::eof)) {
                P->Diag(P->Tok, diag::err_class_block_unclosed);
                break;
            }

            // Parse Modifiers
            llvm::SmallVector<ASTModifier *, 8> Modifiers = P->ParseModifiers();

            // Check if this is a method (identifier followed by parenthesis)
            // Methods are implicitly void - no return type declaration
            // Constructors are identified by name matching class name
            if (P->Tok.isAnyIdentifier()) {
                const StringRef &Name = P->Tok.getIdentifierInfo()->getName();
                const SourceLocation &Loc = P->Tok.getLocation();

                // Look ahead to see if this is a method (has parenthesis)
                Optional<Token> NextTok = Lexer::findNextToken(Loc, P->SourceMgr);
                if (NextTok && NextTok->is(tok::l_paren)) {
                    // This is a method - consume name and parse as method
                    P->ConsumeToken();
                    // Methods are implicitly void - no return type needed
                    ParseMethod(Modifiers, Loc, Name);
                    Continue = true;
                    continue;
                }
            }

            // Otherwise, parse as attribute (Type Name)
            ASTType *Type = P->ParseType();

            Continue = Type != nullptr;
            if (Continue && P->Tok.isAnyIdentifier()) {
                const StringRef &Name = P->Tok.getIdentifierInfo()->getName();
                const SourceLocation &Loc = P->ConsumeToken();

                ParseAttribute(Modifiers, Type, Loc, Name);
                Continue = true;
            }

        } while (Continue);
    }
}

/**
 * ParseModule Class Declaration
 * @return
 */
ASTClass *ParserClass::Parse(Parser *P, SmallVector<ASTModifier *, 8> &Modifiers) {
	FLY_DEBUG_START("ClassParser", "Parse");
    ParserClass *CP = new ParserClass(P, Modifiers);
    ASTClass *Class = CP->Class;
    delete CP;
    return Class;
}

ASTAttribute *ParserClass::ParseAttribute(SmallVector<ASTModifier *, 8> &Modifiers, ASTType *TypeRef, const SourceLocation &Loc, llvm::StringRef Name) {
	FLY_DEBUG_START("ClassParser", "ParseAttribute");

    if (!TypeRef) {
        P->Diag(diag::err_parser_invalid_type);
        return nullptr;
    }

    // Parsing =
    ASTExpr *Expr = nullptr;
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();
        Expr = P->ParseExpr();
    }

    return ASTBuilder::CreateClassAttribute(Loc, Class, TypeRef, Name, Modifiers, Expr);
}

ASTMethod *ParserClass::ParseMethod(SmallVector<ASTModifier *, 8> &Modifiers,
	const SourceLocation &Loc, llvm::StringRef Name) {
	FLY_DEBUG_START("ClassParser", "ParseMethod");

	SmallVector<ASTParam *, 8> Params = ParserFunction::ParseParams(P);
	ASTMethod *Method = ASTBuilder::CreateClassMethod(Loc, Class, Name, Modifiers, Params);
	ASTBlockStmt *Body = P->isBlockStart() ? ParserFunction::ParseBody(P, Method) : nullptr;
	return Method;
}
