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
ClassParser::ClassParser(Parser *P, ASTTopScopes *Scopes) : P(P) {
    assert(P->Tok.isAnyIdentifier() && "Tok must be an Identifier");

    IdentifierInfo *Id = P->Tok.getIdentifierInfo();
    llvm::StringRef Name = Id->getName();
    const SourceLocation Loc = P->Tok.getLocation();
    P->ConsumeToken();

    Class = P->Builder.CreateClass(P->Node, Loc, Name.str(), Scopes);

    do {
        ASTClassScopes *ClassScopes = ParseScopes();
        if (isField()) {
            Success =  ParseField();
        } else if (isMethod()) {
            Success = ParseMethod();
        }
    } while (Success || P->Tok.isNot(tok::eof));
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
    return nullptr;
}

bool ClassParser::isField() {
    return true;
}

ASTClassField *ClassParser::ParseField() {
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

//    ASTGlobalVar *GlobalVar = P->Builder.CreateGlobalVar(Node, Loc, Type, Name.str(), Visibility, Constant);

    // Parsing =
    ASTExpr *Expr = nullptr;
    if (P->isTokenAssign()) {
        P->ConsumeToken();
        Expr = P->ParseExpr();
    }

//    return P->Builder.AddClassField(Node, GlobalVar, Expr) &&
//            P->Builder.AddComment(GlobalVar, Comment);
}

bool ClassParser::isMethod() {
    return true;
}

ASTClassMethod *ClassParser::ParseMethod() {
    return nullptr;
}
