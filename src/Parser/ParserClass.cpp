//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/ClassParser.cpp - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "Parser/ParserMethod.h"
#include "Parser/ParserClass.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTType.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTAssignmentStmt.h"
#include "Sema/SemaBuilder.h"
#include "Basic/Debug.h"

using namespace fly;

/**
 * ClassParser Constructor
 * @param P
 * @param Visibility
 * @param Constant
 */
ParserClass::ParserClass(Parser *P, SmallVector<ASTScope *, 8> &Scopes) : P(P) {
    FLY_DEBUG_MESSAGE("ClassParser", "ClassParser", Logger()
            .AttrList("Scopes", Scopes).End());

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
    // class Example : SuperClass Interface Struct { ... }
    llvm::SmallVector<ASTClassType *, 4> SuperClasses;
    if (P->Tok.is(tok::colon)) {
        P->ConsumeToken();
        while (P->Tok.isAnyIdentifier()) {
            ASTClassType *ClassType = P->Builder.CreateClassType(P->ParseIdentifier());
            SuperClasses.push_back(ClassType);
        }
    }

    // Parse block in the braces
    if (P->isBlockStart()) {
        P->ConsumeBrace(BraceCount);

        Class = P->Builder.CreateClass(P->Module, ClassLoc, ClassKind, ClassName, Scopes, SuperClasses);
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

            // Parse Scopes
            llvm::SmallVector<ASTScope *, 8> Scopes = P->ParseScopes();

            // Parse Type
            ASTType *Type = P->ParseType(); // Continue loop if there is a field or a method

            Continue = Type != nullptr;
            if (Continue && P->Tok.isAnyIdentifier()) {
                const StringRef &Name = P->Tok.getIdentifierInfo()->getName();
                const SourceLocation &Loc = P->ConsumeToken();

                if (P->Tok.is(tok::l_paren)) {
                    ParseMethod(Scopes, Type, Loc, Name);
                } else {
                    ParseAttribute(Scopes, Type, Loc, Name);
                }
                Continue = true;
            }

        } while (Continue);
    }
}

/**
 * ParseModule Class Declaration
 * @return
 */
ASTClass *ParserClass::Parse(Parser *P, SmallVector<ASTScope *, 8> &Scopes) {
    FLY_DEBUG_MESSAGE("ClassParser", "ParseModule", Logger()
            .AttrList("Scopes", Scopes).End());
    ParserClass *CP = new ParserClass(P, Scopes);
    ASTClass *Class = CP->Class;
    delete CP;
    return Class;
}

ASTClassAttribute *ParserClass::ParseAttribute(SmallVector<ASTScope *, 8> &Scopes, ASTType *Type, const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("ClassParser", "ParseMethod", Logger()
            .AttrList("Scopes", Scopes)
            .Attr("Type", Type).End());

    if (!Type) {
        P->Diag(diag::err_parser_invalid_type);
        return nullptr;
    }

    // Parsing =
    ASTExpr *Expr = nullptr;
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();
        Expr = P->ParseExpr();
    }

    return P->Builder.CreateClassAttribute(Loc, *Class, Type, Name, Scopes, Expr);
}

ASTClassMethod *ParserClass::ParseMethod(SmallVector<ASTScope *, 8> &Scopes, ASTType *Type, const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("ClassParser", "ParseMethod", Logger()
            .AttrList("Scopes", Scopes)
            .Attr("Type", Type).End());
    return ParserMethod::Parse(this, Scopes, Type, Loc, Name);
}
