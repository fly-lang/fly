//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/EnumParser.cpp - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/ParserEnum.h"
#include "Parser/Parser.h"
#include "Sema/ASTBuilder.h"
#include "AST/ASTScopes.h"
#include "Basic/Debug.h"

using namespace fly;

/**
 * ClassParser Constructor
 * @param P
 * @param Visibility
 * @param Constant
 */
ParserEnum::ParserEnum(Parser *P, llvm::SmallVector<ASTScope *, 8> &Scopes) : P(P) {
    FLY_DEBUG_START("EnumParser", "ParserEnum");
    assert(P->Tok.is(tok::kw_enum) && "No ClassKind defined");

    P->ConsumeToken();

    // Parse class name
    llvm::StringRef EnumName = P->Tok.getIdentifierInfo()->getName();
    const SourceLocation ClassLoc = P->Tok.getLocation();
    P->ConsumeToken();

    // Parse classes after colon
    // class Example : SuperClass Interface Struct { ... }
    llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
    if (P->Tok.is(tok::colon)) {
        P->ConsumeToken();
        while (P->Tok.isAnyIdentifier()) {
            ASTTypeRef *EnumTypeRef = P->ParseTypeRef();
            SuperClasses.push_back(EnumTypeRef);
        }
    }

    // Parse block in the braces
    if (P->isBlockStart()) {
        P->ConsumeBrace(BraceCount);

        Enum = P->Builder.CreateEnum(P->Module, ClassLoc, EnumName, Scopes, SuperClasses);
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

            SmallVector<ASTScope *, 8> Scopes = P->ParseScopes();

            if (P->Tok.isAnyIdentifier()) {
                const StringRef &Name = P->Tok.getIdentifierInfo()->getName();
                const SourceLocation &Loc = P->ConsumeToken();
                
                Success = ParseEntry(Loc, Name, Scopes);
            }
        } while (Success);
    }
}

/**
 * ParseModule Class Declaration
 * @return
 */
ASTEnum *ParserEnum::Parse(Parser *P, SmallVector<ASTScope *, 8> &Scopes) {
	FLY_DEBUG_START("EnumParser", "Parse");
    ParserEnum *CP = new ParserEnum(P, Scopes);
    return CP->Enum;
}

bool ParserEnum::ParseEntry(const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> Scopes) {
	FLY_DEBUG_START("EnumParser", "ParserEntry");
    ASTVar *EnumEntry = P->Builder.CreateEnumEntry(Loc, Enum, Name, Scopes);
    return true;
}
