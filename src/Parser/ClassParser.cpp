//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/ClassParser.cpp - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "Parser/FunctionParser.h"
#include "Parser/ClassParser.h"
#include "AST/ASTBlock.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"
#include "Sema/SemaBuilder.h"
#include "Basic/Debug.h"

using namespace fly;

/**
 * ClassParser Constructor
 * @param P
 * @param Visibility
 * @param Constant
 */
ClassParser::ClassParser(Parser *P, ASTTopScopes *Scopes) : P(P) {
    assert(P->Tok.isAnyIdentifier() && "Tok must be an Identifier");

    FLY_DEBUG_MESSAGE("ClassParser", "ClassParser", Logger()
            .Attr("Scopes", Scopes).End());

    llvm::StringRef ClassName = P->Tok.getIdentifierInfo()->getName();
    const SourceLocation ClassLoc = P->Tok.getLocation();
    P->ConsumeToken();

    if (P->isBlockStart()) {
        ConsumeBrace();

        Class = P->Builder.CreateClass(P->Node, ClassLoc, ClassName, Scopes);
        bool Continue;
        do {

            // End of the Class
            if (P->isBlockEnd() ) {
                ConsumeBrace();
                break;
            }

            // Error: Class block not correctly closed
            if (P->Tok.is(tok::eof)) {
                Success = false;
                P->Diag(P->Tok, diag::err_class_block_unclosed);
                break;
            }

            // Parse Scopes
            ASTClassScopes *ClassScopes = ParseScopes();

            // Parse Type
            ASTType *Type = P->ParseType();
            Continue = false; // Continue loop if there is a field or a method
            if (P->Tok.isAnyIdentifier()) {
                const StringRef &Name = P->Tok.getIdentifierInfo()->getName();
                const SourceLocation &Loc = P->ConsumeToken();

                if (P->Tok.is(tok::l_paren)) {
                    Success &= ParseMethod(ClassScopes, Type, Loc, Name);
                } else {
                    Success &= ParseField(ClassScopes, Type, Loc, Name);
                }
                Continue = true;
            }

        } while (Continue);
    }
}

/**
 * Parse Class Declaration
 * @return
 */
ASTClass *ClassParser::Parse(Parser *P, ASTTopScopes *Scopes) {
    FLY_DEBUG_MESSAGE("ClassParser", "Parse", Logger()
            .Attr("Scopes", Scopes).End());
    ClassParser *CP = new ClassParser(P, Scopes);
    return CP->Class;
}

ASTClassScopes *ClassParser::ParseScopes() {
    FLY_DEBUG("ClassParser", "ParseScopes");

    bool isPrivate = false;
    bool isPublic = false;
    bool isConst = false;
    bool Found;
    do {
        if (P->Tok.is(tok::kw_private)) {
            if (isPrivate) {
                P->Diag(P->Tok, diag::err_scope_visibility_duplicate << (int) ASTClassVisibilityKind::CLASS_V_PRIVATE);
            }
            if (isPublic) {
                P->Diag(P->Tok, diag::err_scope_visibility_conflict
                    << (int) ASTClassVisibilityKind::CLASS_V_PRIVATE << (int) ASTClassVisibilityKind::CLASS_V_PUBLIC);
            }
            isPrivate = true;
            Found = true;
            P->ConsumeToken();
        } else if (P->Tok.is(tok::kw_public)) {
            if (isPublic) {
                P->Diag(P->Tok, diag::err_scope_visibility_conflict
                    << (int) ASTClassVisibilityKind::CLASS_V_PUBLIC << (int) ASTClassVisibilityKind::CLASS_V_PUBLIC);
            }
            if (isPrivate) {
                P->Diag(P->Tok, diag::err_scope_visibility_duplicate
                    << (int) ASTClassVisibilityKind::CLASS_V_PRIVATE);
            }
            isPublic = true;
            Found = true;
            P->ConsumeToken();
        } else if (P->Tok.is(tok::kw_const)) {
            if (isConst) {
                P->Diag(P->Tok, diag::err_scope_const_duplicate);
            }
            isConst = true;
            Found = true;
            P->ConsumeToken();
        } else {
            Found = false;
        }
    } while (Found);

    ASTClassVisibilityKind Visibility = isPrivate ? ASTClassVisibilityKind::CLASS_V_PRIVATE :
            (isPublic ? ASTClassVisibilityKind::CLASS_V_PUBLIC : ASTClassVisibilityKind::CLASS_V_DEFAULT);
    return SemaBuilder::CreateClassScopes(Visibility, isConst);
}

bool ClassParser::ParseField(ASTClassScopes *Scopes, ASTType *Type, const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("ClassParser", "ParseMethod", Logger()
            .Attr("Scopes", Scopes)
            .Attr("Type", Type).End());

    if (!Type) {
        P->Diag(diag::err_parser_invalid_type);
        return false;
    }

    // Add Comment to AST
    llvm::StringRef Comment;
    if (!P->BlockComment.empty()) {
        Comment = P->BlockComment;
    }

    ASTClassVar *ClassVar = P->Builder.CreateClassVar(Class, Loc, Type, Name, Scopes);
    if (ClassVar) {
        // Parsing =
        if (P->Tok.is(tok::equal)) {
            P->ConsumeToken();
            ASTExpr *Expr = P->ParseExpr();
            ClassVar->setExpr(Expr);
        }

        return P->Builder.AddClassVar(Class, ClassVar) && P->Builder.AddComment(ClassVar, Comment);
    }

    return false;
}

bool ClassParser::ParseMethod(ASTClassScopes *Scopes, ASTType *Type, const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("ClassParser", "ParseMethod", Logger()
            .Attr("Scopes", Scopes)
            .Attr("Type", Type).End());

    // Add Comment to AST
    llvm::StringRef Comment;
    if (!P->BlockComment.empty()) {
        Comment = P->BlockComment;
    }

    ASTClassFunction *Method;
    if (Name == Class->getName()) {
        if (!Type) {
            Method = P->Builder.CreateClassConstructor(Class, Loc, Scopes);
            Success = FunctionParser::Parse(P, Method) && P->Builder.AddClassConstructor(Class, Method);
        } else {
            P->Diag(diag::err_parser_invalid_type);
            return false;
        }
    } else {
        Method = P->Builder.CreateClassMethod(Class, Loc, Type, Name, Scopes);
        Success = FunctionParser::Parse(P, Method) && P->Builder.AddClassMethod(Class, Method);
    }

    if (Method && Method->getBody()->isEmpty()) {
        Method->Abstract = true;
    }

    return Success;
}

/**
 * ConsumeBrace - This consume method keeps the brace count up-to-date.
 * @return
 */
SourceLocation ClassParser::ConsumeBrace() {
    FLY_DEBUG("Parser", "ConsumeBrace");
    assert(P->isTokenBrace() && "wrong consume method");
    if (P->Tok.getKind() == tok::l_brace)
        ++BraceCount;
    else if (BraceCount) {
        //AngleBrackets.clear(*this);
        --BraceCount;     // Don't let unbalanced }'s drive the count negative.
    }

    return P->ConsumeNext();
}

bool ClassParser::isBraceBalanced() const {
    FLY_DEBUG("Parser", "isBraceBalanced");
    return BraceCount == 0;
}
