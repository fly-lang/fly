//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/EnumParser.cpp - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/ParserEnum.h"

#include "AST/ASTBuilder.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTModifier.h"
#include "Basic/Debug.h"
#include "Parser/Parser.h"

using namespace fly;

/**
 * ClassParser Constructor
 * @param P
 * @param Visibility
 * @param Constant
 */
ParserEnum::ParserEnum(Parser *P, llvm::SmallVector<ASTModifier *, 8> &Modifiers) : P(P) {
    FLY_DEBUG_SCOPE("EnumParser", "ParserEnum");
    assert(P->Tok.is(tok::kw_enum) && "No ClassKind defined");

    P->ConsumeToken();

    // Parse class name
    llvm::StringRef EnumName = P->Tok.getIdentifierInfo()->getName();
    const SourceLocation ClassLoc = P->Tok.getLocation();
    P->ConsumeToken();

    // Parse classes after colon
    // class Example : SuperClass Interface Struct { ... }
    llvm::SmallVector<ASTType *, 4> Bases;
    if (P->Tok.is(tok::colon)) {
        P->ConsumeToken();
        while (P->Tok.isAnyIdentifier()) {
            ASTType *T = P->ParseType();
            Bases.push_back(T);
        }
    }

    // Parse block in the braces
    if (P->isBlockStart()) {
        P->ConsumeBrace(BraceCount);

        Enum = ASTBuilder::CreateEnum(P->Module, ClassLoc, EnumName, Modifiers, Bases);
        do {

            // End of the Class
            if (P->isBlockEnd() ) {
                P->ConsumeBrace(BraceCount);
                break;
            }

            // Error: Class block not correctly closed
            if (P->Tok.is(tok::eof)) {
                Success = false;
                P->Diag(P->Tok, diag::err_parser_class_block_unclosed);
                break;
            }

            // Parse any modifiers for the next enum entries (if present).
            SmallVector<ASTModifier *, 8> ParsedMods = P->ParseModifiers();

            if (P->Tok.isAnyIdentifier()) {
                const StringRef &Name = P->Tok.getIdentifierInfo()->getName();
                const SourceLocation &Loc = P->ConsumeToken();

                Success = ParseEntry(Loc, Name, ParsedMods);
            }
        } while (Success);
    }
}

/**
 * ParseModule Class Declaration
 * @return
 */
ASTEnum *ParserEnum::Parse(Parser *P, SmallVector<ASTModifier *, 8> &Modifiers) {
	FLY_DEBUG_SCOPE("EnumParser", "Parse");
    ParserEnum *PE = new ParserEnum(P, Modifiers);
    return PE->Enum;
}

bool ParserEnum::ParseEntry(const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> Modifiers) {
     FLY_DEBUG_SCOPE("EnumParser", "ParserEntry");
     // Create the first entry
     ASTEnumEntry *EnumEntry = ASTBuilder::CreateEnumEntry(Loc, Enum, Name, Modifiers);

     // Allow comma separated entries on the same line: A, B, C
    while (P->Tok.is(tok::comma)) {
        // consume comma
        P->ConsumeToken();

        // If comma is trailing (e.g. before the block end) accept it and finish.
        if (P->isBlockEnd() || P->Tok.is(tok::r_brace)) {
            break;
        }

        // Expect another identifier after comma
        if (!P->Tok.isAnyIdentifier()) {
            // malformed list: no identifier after comma
            P->Diag(P->Tok, diag::err_parser_identifier_expected);
            return false;
        }

        // Create the next enum entry
        Name = P->Tok.getIdentifierInfo()->getName();
        const SourceLocation NextLoc = P->ConsumeToken();
        ASTBuilder::CreateEnumEntry(NextLoc, Enum, Name, Modifiers);
    }

     return true;
  }
