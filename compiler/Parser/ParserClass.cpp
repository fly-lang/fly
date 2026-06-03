//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Parser/ParserClass.cpp - class declaration parser
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
#include "AST/ASTType.h"
#include "Basic/Debug.h"
#include "Basic/TokenKinds.h"

#include <AST/ASTMethod.h>

using namespace fly;

/**
 * ClassParser Constructor
 * @param P
 * @param Visibility
 * @param Constant
 */
ParserClass::ParserClass(Parser *P, SmallVector<ASTModifier *, 8> &Modifiers, bool SkipBodies) : P(P), SkipBodies(SkipBodies) {
    FLY_DEBUG_SCOPE("ClassParser", "ClassParser");

    ASTClassKind ClassKind;
    if (P->Tok.is(tok::kw_struct)) {
        ClassKind = ASTClassKind::STRUCT;
    } else if (P->Tok.is(tok::kw_class)) {
        ClassKind = ASTClassKind::CLASS;
    } else if (P->Tok.is(tok::kw_interface)) {
        ClassKind = ASTClassKind::INTERFACE;
    } else if (P->Tok.is(tok::kw_suite)) {
        ClassKind = ASTClassKind::SUITE;
    } else {
        assert("No ClassKind defined");
    }
    P->ConsumeToken();

    // Parse class name
    llvm::StringRef ClassName = P->Tok.getIdentifierInfo()->getName();
    const SourceLocation ClassLoc = P->Tok.getLocation();
    P->ConsumeToken();

    // Parse optional generic type parameters: class Foo<T, U : Bar>
    llvm::SmallVector<ASTTypeParam *, 4> TypeParams;
    if (P->Tok.is(tok::less)) {
        TypeParams = P->ParseTypeParams();
        // Generic classes must keep their bodies so specializations can be code-generated.
        this->SkipBodies = false;
    }

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
        if (!TypeParams.empty()) {
            Class->TypeParams = TypeParams;
        }
        bool Continue;
        do {

            // End of the Class
            if (P->isBlockEnd() ) {
                P->ConsumeBrace(BraceCount);
                break;
            }

            // Error: Class block not correctly closed
            if (P->Tok.is(tok::eof)) {
                P->Diag(P->Tok, diag::err_parser_class_block_unclosed);
                break;
            }

            // Parse Modifiers
            llvm::SmallVector<ASTModifier *, 8> Modifiers = P->ParseModifiers();

            // Method with return type: "int|bool|... name(params) { body }"
            // Detect: builtin-type identifier '('
            if (P->isBuiltinType(P->Tok)) {
                std::optional<Token> AfterType = Lexer::findNextToken(P->Tok.getLocation(), P->SourceMgr);
                if (AfterType && AfterType->isAnyIdentifier()) {
                    std::optional<Token> AfterName = Lexer::findNextToken(AfterType->getLocation(), P->SourceMgr);
                    if (AfterName && AfterName->is(tok::l_paren)) {
                        // Method with return type
                        ASTType *RetType = P->ParseType();
                        const StringRef &Name = P->Tok.getIdentifierInfo()->getName();
                        const SourceLocation &Loc = P->Tok.getLocation();
                        P->ConsumeToken();
                        ASTMethod *M = ParseMethod(Modifiers, Loc, Name);
                        if (M) {
                            bool IsVoid = RetType->getTypeKind() == ASTTypeKind::TYPE_BUILTIN &&
                                          static_cast<ASTBuiltinType *>(RetType)->getBuiltinKind() == ASTBuiltinTypeKind::TYPE_VOID;
                            if (!IsVoid) {
                                M->setReturnType(RetType);
                            }
                        }
                        Continue = true;
                        continue;
                    }
                }
                // Fall through: builtin type used as attribute type
            }

            // Named type return: "public Time now() { … }" or "public fly.os.io.Buf read() { … }"
            // Must verify token after method name is '(' to distinguish from field declarations.
            else if (P->Tok.isAnyIdentifier()) {
                // Scan past the qualified type name to find the method name token
                SourceLocation TLoc = P->Tok.getLocation();
                std::optional<Token> TNext = Lexer::findNextToken(TLoc, P->SourceMgr);
                while (TNext && TNext->is(tok::period)) {
                    std::optional<Token> AfterDot = Lexer::findNextToken(TNext->getLocation(), P->SourceMgr);
                    if (AfterDot && AfterDot->isAnyIdentifier()) {
                        TLoc = AfterDot->getLocation();
                        TNext = Lexer::findNextToken(TLoc, P->SourceMgr);
                    } else break;
                }
                // TNext is now the candidate method name token; check it is an identifier
                // followed immediately by '('
                if (TNext && TNext->isAnyIdentifier()) {
                    std::optional<Token> AfterName = Lexer::findNextToken(TNext->getLocation(), P->SourceMgr);
                    if (AfterName && AfterName->is(tok::l_paren)) {
                        ASTType *RetType = P->ParseType(); // consumes the full type name
                        const StringRef &Name = P->Tok.getIdentifierInfo()->getName();
                        const SourceLocation &Loc = P->Tok.getLocation();
                        P->ConsumeToken();
                        ASTMethod *M = ParseMethod(Modifiers, Loc, Name);
                        if (M) {
                            M->setReturnType(RetType);
                        }
                        Continue = true;
                        continue;
                    }
                }
            }

            // Check if this is a method without a return type (identifier followed by '(')
            // This is now an error: all methods must declare a return type.
            // Constructors are the exception — they are identified by name matching the class name
            // and are still allowed without a return type.
            if (P->Tok.isAnyIdentifier()) {
                const StringRef &Name = P->Tok.getIdentifierInfo()->getName();
                const SourceLocation &Loc = P->Tok.getLocation();

                // Look ahead to see if this is a method (has parenthesis)
                std::optional<Token> NextTok = Lexer::findNextToken(Loc, P->SourceMgr);
                if (NextTok && NextTok->is(tok::l_paren)) {
                    // Suites have no constructors; all methods must have a return type
                    bool IsConstructor = (Class->getClassKind() != ASTClassKind::SUITE) &&
                                         (Name == Class->getName());
                    if (!IsConstructor) {
                        // Non-constructor method without a return type — emit error and recover
                        P->Diag(Loc, diag::err_parser_missing_return_type);
                    }
                    P->ConsumeToken();
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
ASTClass *ParserClass::Parse(Parser *P, SmallVector<ASTModifier *, 8> &Modifiers, bool SkipBodies) {
	FLY_DEBUG_SCOPE("ClassParser", "Parse");
    ParserClass *CP = new ParserClass(P, Modifiers, SkipBodies);
    ASTClass *Class = CP->Class;
    delete CP;
    return Class;
}

ASTAttribute *ParserClass::ParseAttribute(SmallVector<ASTModifier *, 8> &Modifiers, ASTType *TypeRef, const SourceLocation &Loc, llvm::StringRef Name) {
	FLY_DEBUG_SCOPE("ClassParser", "ParseAttribute");

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
	FLY_DEBUG_SCOPE("ClassParser", "ParseMethod");

	SmallVector<ASTParam *, 8> Params = ParserFunction::ParseParams(P);
	ASTMethod *Method = ASTBuilder::CreateClassMethod(Loc, Class, Name, Modifiers, Params);
	if (P->isBlockStart()) {
		if (SkipBodies) {
			// Create an empty non-null body so Sema doesn't treat this method as
			// abstract (Body==nullptr signals an abstract/unimplemented method).
			ASTBlockStmt *EmptyBlock = ASTBuilder::CreateBlockStmt(P->Tok.getLocation());
			ASTBuilder::CreateBody(Method, EmptyBlock);
			P->SkipBraceBlock();
		} else {
			ParserFunction::ParseBody(P, Method);
		}
	}
	return Method;
}
