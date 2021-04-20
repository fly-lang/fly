//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/Parser.cpp - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"

using namespace fly;

Parser::Parser(Lexer &Lex, DiagnosticsEngine &Diags) : Lex(Lex), Diags(Diags) {
}

bool Parser::Parse(ASTNode *Node) {
    AST = Node;
    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();

    // Parse Package on first
    if (ParseNameSpace()) {

        // Parse Imports
        if (ParseImportDecl()) {

            // Parse All
            while (Tok.isNot(tok::eof)) {
                if (!ParseTopDecl()) {
                    return false;
                }
            }

            return AST->Finalize();
        }
    }

    return false;
}

DiagnosticBuilder Parser::Diag(SourceLocation Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

DiagnosticBuilder Parser::Diag(const Token &Tok, unsigned DiagID) {
    return Diag(Tok.getLocation(), DiagID);
}

/**
 * Parse package declaration
 * @param fileName
 * @return true on Succes or false on Error
 */
bool Parser::ParseNameSpace() {
    // Check for first declaration
    if (Tok.is(tok::kw_namespace)) {
        SourceLocation StartLoc = Tok.getLocation();
        SourceLocation PackageNameLoc = ConsumeToken();
        if (Tok.isLiteral()) {
            StringRef Name = getLiteralString();
            AST->setNameSpace(Name);
            return true;
        }

        Diag(Tok, diag::err_package_undefined);
        return false;

    }

    // Define Default NameSpace if it is not defined
    AST->setNameSpace();
    return true;
}

/**
 * Parse import declarations
 * @return
 */
bool Parser::ParseImportDecl() {
    if (Tok.is(tok::kw_import)) {
        SourceLocation AfterImportDeclLoc = ConsumeToken();
        if (Tok.is(tok::l_paren)) {
            ConsumeParen();

            // Parse import ( ... ) declarations
            return ParseImportParenDecl();

        } else if (Tok.isLiteral()) {

            // Parse import "..."
            StringRef Name = getLiteralString();

            // Syntax Error Quote
            if (Name.empty()) {
                Diag(Tok, diag::err_package_undefined);
                return false;
            }

            if (ParseImportAliasDecl(Name)) {
                return ParseImportDecl();
            }
        }
    }

    // if parse "package" there is multiple package declarations
    if (Tok.is(tok::kw_namespace)) {

        // Multiple Package declaration is invalid, you can define only one
        Diag(Tok, diag::err_package_undefined);
        return false;
    }

    return true;
}

StringRef Parser::getLiteralString() {
    StringRef Name(Tok.getLiteralData(), Tok.getLength());
    if (Name.startswith("\"") && Name.endswith("\"")) {
        StringRef StrRefName = Name.substr(1, Name.size()-2);
        ConsumeStringToken();
        return StrRefName;
    }

    return "";
}

bool Parser::ParseImportParenDecl() {
    if (Tok.isLiteral()) {
        StringRef Name = getLiteralString();
        if (Name.empty()) {
            Diag(Tok, diag::err_package_undefined);
            return false;
        }

        if (!ParseImportAliasDecl(Name)) {
            return false;
        }


        if (Tok.is(tok::comma)) {
            ConsumeNext();
            return ParseImportParenDecl();
        }

        if (Tok.is(tok::r_paren)) {
            ConsumeParen();
            return true;
        }
    }

    return true;
}

bool Parser::ParseImportAliasDecl(StringRef Name) {
    if (Tok.is(tok::kw_as)) {
        ConsumeToken();
        if (Tok.isLiteral()) {
            StringRef Alias = getLiteralString();
            return AST->addImport(Name, Alias);
        }
    }
    return AST->addImport(Name, "");
}

bool Parser::ParseTopDecl() {

    // Visibility Kind
    VisibilityKind Visibility;
    if (Tok.is(tok::kw_private)) {
        Visibility = VisibilityKind::Private;
        ConsumeToken();
    } else if (Tok.is(tok::kw_public)) {
        Visibility = VisibilityKind::Public;
        ConsumeToken();
    } else {
        Visibility = VisibilityKind::Default;
    }

    // Modifiable Kind
    ModifiableKind Modifiable;
    if (Tok.is(tok::kw_const)) {
        Modifiable = ModifiableKind::Constant;
        ConsumeToken();
    } else {
        Modifiable = ModifiableKind::Variable;
    }

    // Var or Function
    if (Tok.isTypeIdentifier()) {

        Token TypeToken = Tok;
        SourceLocation TypeLoc = Tok.getLocation();
        ConsumeToken();

        if (Tok.isAnyIdentifier()) {
            IdentifierInfo *Info = Tok.getIdentifierInfo();
            Token IdentifierToken = Tok;
            SourceLocation IdentifierLoc = Tok.getLocation();
            ConsumeToken();
            if (Tok.is(tok::l_paren)) {
                return ParseFunctionDecl();
            }
            return ParseGlobalVarDecl(Visibility, Modifiable, TypeToken, TypeLoc, Info, IdentifierLoc);
        }

        // Check Error: type without identifier
        return false;
    }
}

bool Parser::ParseGlobalVarDecl(VisibilityKind Visibility, ModifiableKind Modifiable,
                                Token TypeToken, SourceLocation TypeLoc, IdentifierInfo *Info, SourceLocation IdLoc) {
    GlobalVarDecl* VarPtr = nullptr;
    switch (TypeToken.getKind()) {
        case tok::kw_bool:
            VarPtr = AST->addBoolVar(Visibility, Modifiable, Info->getName());
            break;
        case tok::kw_int:
            VarPtr = AST->addIntVar(Visibility, Modifiable, Info->getName());
            break;
        case tok::kw_float:
            VarPtr = AST->addFloatVar(Visibility, Modifiable, Info->getName());
            break;
    }
    return VarPtr != nullptr;
}

bool Parser::ParseFunctionDecl() {
    return false;
}

ASTNode *Parser::getAST() {
    return AST;
}
