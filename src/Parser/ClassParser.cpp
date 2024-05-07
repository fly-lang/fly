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
#include "AST/ASTBlockStmt.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTType.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTVarStmt.h"
#include "Sema/SemaBuilder.h"
#include "Basic/Debug.h"

using namespace fly;

/**
 * ClassParser Constructor
 * @param P
 * @param Visibility
 * @param Constant
 */
ClassParser::ClassParser(Parser *P, ASTScopes *ClassScopes) : P(P) {
    FLY_DEBUG_MESSAGE("ClassParser", "ClassParser", Logger()
            .Attr("Scopes", ClassScopes).End());

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

        Class = P->Builder.CreateClass(ClassLoc, ClassKind, ClassName, ClassScopes, SuperClasses);
        bool Continue;
        do {

            // End of the Class
            if (P->isBlockEnd() ) {
                P->ConsumeBrace(BraceCount);
                break;
            }

            // Error: Class block not correctly closed
            if (P->Tok.is(tok::eof)) {
                Success = false;
                P->Diag(P->Tok, diag::err_class_block_unclosed);
                break;
            }

            // Parse Scopes
            ASTScopes *Scopes = P->Builder.CreateScopes();

            // Parse Type
            ASTType *Type = nullptr;

            Continue = P->ParseScopes(Scopes) && P->ParseType(Type); // Continue loop if there is a field or a method
            if (Continue && P->Tok.isAnyIdentifier()) {
                const StringRef &Name = P->Tok.getIdentifierInfo()->getName();
                const SourceLocation &Loc = P->ConsumeToken();

                if (P->Tok.is(tok::l_paren)) {
                    Success &= ParseMethod(Scopes, Type, Loc, Name);
                } else {
                    Success &= ParseField(Scopes, Type, Loc, Name);
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
ASTClass *ClassParser::Parse(Parser *P, ASTScopes *ClassScopes) {
    FLY_DEBUG_MESSAGE("ClassParser", "Parse", Logger()
            .Attr("Scopes", ClassScopes).End());
    ClassParser *CP = new ClassParser(P, ClassScopes);
    return CP->Class;
}

bool ClassParser::ParseField(ASTScopes *Scopes, ASTType *Type, const SourceLocation &Loc, llvm::StringRef Name) {
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

    ASTClassAttribute *ClassVar = P->Builder.CreateClassAttribute(Loc, Type, Name, Scopes);
    if (ClassVar) {
        // Parsing =
        if (P->Tok.is(tok::equal)) {
            P->ConsumeToken();

            ASTVarStmt *VarDefine = P->Builder.CreateVarStmt(ClassVar);
            ASTExpr *Expr = P->ParseExpr();
            VarDefine->setExpr(Expr);
            ClassVar->setInit(VarDefine);
        }

        return P->Builder.AddClassAttribute(Class, ClassVar) && P->Builder.AddComment(ClassVar, Comment);
    }

    return false;
}

bool ClassParser::ParseMethod(ASTScopes *Scopes, ASTType *Type, const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("ClassParser", "ParseMethod", Logger()
            .Attr("Scopes", Scopes)
            .Attr("Type", Type).End());

    // Add Comment to AST
    llvm::StringRef Comment;
    if (!P->BlockComment.empty()) {
        Comment = P->BlockComment;
    }

    ASTClassMethod *Method;
    if (Name == Class->getName()) {
        if (!Type) {
            Method = P->Builder.CreateClassConstructor(Loc, Scopes);
            Success = FunctionParser::Parse(P, Method) && P->Builder.AddClassMethod(Class, Method);
        } else {
            P->Diag(diag::err_parser_invalid_type);
            Success = false;
        }
    } else {
        Method = P->Builder.CreateClassMethod(Loc, Type, Name, Scopes);
        Success = FunctionParser::Parse(P, Method) && P->Builder.AddClassMethod(Class, Method);
    }

    return Success;
}
