//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/ClassParser.cpp - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "Parser/ClassParser.h"
#include "AST/ASTClass.h"
#include "Sema/SemaBuilder.h"
#include "Basic/Debug.h"

using namespace fly;

/**
 * ClassParser Constructor
 * @param P
 * @param Visibility
 * @param Constant
 */
ClassParser::ClassParser(Parser *P, ASTTopScopes *TopScopes) : P(P) {
    assert(P->Tok.isAnyIdentifier() && "Tok must be an Identifier");

    IdentifierInfo *Id = P->Tok.getIdentifierInfo();
    llvm::StringRef Name = Id->getName();
    const SourceLocation Loc = P->Tok.getLocation();
    P->ConsumeToken();

    if (P->isBlockStart()) {
        P->ConsumeBrace();

        Class = P->Builder.CreateClass(P->Node, Loc, Name.str(), TopScopes);
        bool Continue;
        do {

            // End of the Class
            if (P->isBlockEnd() ) {
                P->ConsumeBrace();
                break;
            }

            // Error: Class block not correctly closed
            if (P->Tok.is(tok::eof)) {
                Success = false;
                P->Diag(P->Tok, diag::err_class_block_unclosed);
                break;
            }

            // Parse Scopes
            ASTClassScopes *Scopes = ParseScopes();

            // Parse Type
            ASTType *Type = P->ParseType();
            if (!Type) {
                P->Diag(diag::err_parser_invalid_type);
                Success = false;
            }

            Continue = false; // Continue loop if there is a field or a method
            if (isField()) {
                Success &= ParseField(Scopes, Type);
                Continue = true;
            } else if (isMethod()) {
                Success &= ParseMethod(Scopes, Type);
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
    ClassParser *CP = new ClassParser(P, Scopes);
    return CP->Class;
}

ASTClassScopes *ClassParser::ParseScopes() {
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

bool ClassParser::isField() {
    return P->Tok.isAnyIdentifier();
}

bool ClassParser::ParseField(ASTClassScopes *Scopes, ASTType *Type) {
    FLY_DEBUG("Parser", "ParseGlobalVar");

    assert(P->Tok.isAnyIdentifier() && "Tok must be an Identifier");

    // Add Comment to AST
    std::string Comment;
    if (!P->BlockComment.empty()) {
        Comment = P->BlockComment;
        P->ClearBlockComment(); // Clear for next use
    }

    IdentifierInfo *Id = P->Tok.getIdentifierInfo();
    llvm::StringRef Name = Id->getName();
    SourceLocation Loc = P->ConsumeToken();

    ASTClassVar *Field = P->Builder.CreateClassVar(Class, Loc, Type, Name.str(), Scopes);
    if (Field) {
        // Parsing =
        ASTExpr *Expr = nullptr;
        if (P->isTokenAssign()) {
            P->ConsumeToken();
            Expr = P->ParseExpr();
        }

        return P->Builder.AddComment(Field, Comment);
    }

    return false;
}

bool ClassParser::isMethod() {
    return false;
}

bool ClassParser::ParseMethod(ASTClassScopes *Scopes, ASTType *Type) {
    return false;
}
