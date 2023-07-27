//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/EnumParser.cpp - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/EnumParser.h"
#include "Parser/Parser.h"
#include "AST/ASTType.h"
#include "Sema/SemaBuilder.h"
#include "Basic/Debug.h"

using namespace fly;

/**
 * ClassParser Constructor
 * @param P
 * @param Visibility
 * @param Constant
 */
EnumParser::EnumParser(Parser *P, ASTScopes *EnumScopes) : P(P) {
    FLY_DEBUG_MESSAGE("ClassParser", "ClassParser", Logger()
            .Attr("Scopes", EnumScopes).End());
    assert(P->Tok.is(tok::kw_enum) && "No ClassKind defined");

    P->ConsumeToken();

    // Parse class name
    llvm::StringRef EnumName = P->Tok.getIdentifierInfo()->getName();
    const SourceLocation ClassLoc = P->Tok.getLocation();
    P->ConsumeToken();

    // Parse classes after colon
    // class Example : SuperClass Interface Struct { ... }
    llvm::SmallVector<ASTEnumType *, 4> SuperClasses;
    if (P->Tok.is(tok::colon)) {
        P->ConsumeToken();
        while (P->Tok.isAnyIdentifier()) {
            ASTEnumType *EnumType = P->Builder.CreateEnumType(P->ParseIdentifier());
            SuperClasses.push_back(EnumType);
        }
    }

    // Parse block in the braces
    if (P->isBlockStart()) {
        P->ConsumeBrace(BraceCount);

        Enum = P->Builder.CreateEnum(P->Node, EnumScopes, ClassLoc, EnumName, SuperClasses);
        uint64_t Index = 0;
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

            if (P->Tok.isAnyIdentifier()) {
                const StringRef &Name = P->Tok.getIdentifierInfo()->getName();
                const SourceLocation &Loc = P->ConsumeToken();
                Success = ParseField(Loc, Name);
            }
        } while (Success);
    }
}

/**
 * Parse Class Declaration
 * @return
 */
ASTEnum *EnumParser::Parse(Parser *P, ASTScopes *EnumScopes) {
    FLY_DEBUG_MESSAGE("EnumParser", "Parse", Logger()
            .Attr("Scopes", EnumScopes).End());
    EnumParser *CP = new EnumParser(P, EnumScopes);
    return CP->Enum;
}

bool EnumParser::ParseField(const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("ClassParser", "ParseMethod", Logger().Attr("Type", Name).End());

    ASTEnumVar *EnumVar = P->Builder.CreateEnumVar(Enum, Loc, Name);

    // Add Comment to AST
    llvm::StringRef Comment;
    if (!P->BlockComment.empty()) {
        Comment = P->BlockComment;
    }

    return P->Builder.AddEnumVar(EnumVar);
}
