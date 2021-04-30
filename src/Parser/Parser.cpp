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
    if (ParseNameSpace() && Tok.isNot(tok::eof)) {

        // Parse Imports
        if (ParseImportDecl() && Tok.isNot(tok::eof)) {

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
        if (Tok.isAnyIdentifier()) {
            StringRef Name = Tok.getIdentifierInfo()->getName();
            ConsumeToken();
            AST->setNameSpace(Name);
            return true;
        }

        Diag(Tok, diag::err_namespace_undefined);
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
        const SourceLocation ImportLoc = ConsumeToken();
        if (Tok.is(tok::l_paren)) {
            ConsumeParen();

            // Parse import ( ... ) declarations
            return ParseImportParenDecl();

        } else if (Tok.isLiteral()) {

            // Parse import "..."
            StringRef Name = getLiteralString();

            // Syntax Error Quote
            if (Name.empty()) {
                Diag(Tok, diag::err_namespace_undefined);
                return false;
            }

            if (ParseImportAliasDecl(ImportLoc, Name)) {
                return ParseImportDecl();
            }
        }
    }

    // if parse "package" there is multiple package declarations
    if (Tok.is(tok::kw_namespace)) {

        // Multiple Package declaration is invalid, you can define only one
        Diag(Tok, diag::err_namespace_undefined);
        return false;
    }

    return true;
}

bool Parser::ParseTopDecl() {

    VisibilityKind Visibility = VisibilityKind::V_DEFAULT;
    bool Constant = false;

    // Parse Public or Private and Constant
    if (ParseTopScopes(Visibility, Constant)) {

        if (Tok.is(tok::kw_class)) {
            return ParseClassDecl(Visibility, Constant);
        }

        // Parse Type
        TypeDecl *TyDecl = ParseType();

        if (TyDecl) {

            if (Tok.isAnyIdentifier()) {
                IdentifierInfo *Id = Tok.getIdentifierInfo();
                SourceLocation IdLoc = Tok.getLocation();
                ConsumeToken();
                if (Tok.is(tok::l_paren)) {
                    return ParseFunctionDecl(Visibility, Constant, TyDecl, Id, IdLoc);
                }
                return ParseGlobalVarDecl(Visibility, Constant, TyDecl, Id, IdLoc);
            }
        }
    }

    // Check Error: type without identifier
    return false;
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
        SourceLocation ImportLoc = Tok.getLocation();
        StringRef Name = getLiteralString();
        if (Name.empty()) {
            Diag(Tok, diag::err_namespace_undefined);
            return false;
        }

        if (!ParseImportAliasDecl(ImportLoc, Name)) {
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

bool Parser::ParseImportAliasDecl(const SourceLocation &Location, StringRef Name) {
    if (Tok.is(tok::kw_as)) {
        ConsumeToken();
        if (Tok.isLiteral()) {
            StringRef Alias = getLiteralString();

            return AST->addImport(new ImportDecl(Location, Name, Alias));
        }
    }
    return AST->addImport(new ImportDecl(Location, Name));
}

bool Parser::ParseTopScopes(VisibilityKind &Visibility, bool &Constant) {
    if (Tok.is(tok::kw_private)) {
        Visibility = VisibilityKind::V_PRIVATE;
        ConsumeToken();
        return ParseTopScopes(Visibility, Constant);
    } else if (Tok.is(tok::kw_public)) {
        Visibility = VisibilityKind::V_PUBLIC;
        ConsumeToken();
        return ParseTopScopes(Visibility, Constant);
    } else if (Tok.is(tok::kw_const)) {
        Constant = true;
        ConsumeToken();
        return ParseTopScopes(Visibility, Constant);
    }
    return true;
}

bool Parser::ParseScopes(bool &Constant) {
    if (Tok.is(tok::kw_const)) {
        Constant = true;
        ConsumeToken();
    }
    return true;
}

bool Parser::ParseGlobalVarDecl(VisibilityKind &VisKind, bool &Constant, TypeDecl *TyDecl,
                                IdentifierInfo *Id, SourceLocation &IdLoc) {
    const StringRef &IdName = Id->getName();
    GlobalVarParser Parser(this, TyDecl, IdName, IdLoc);
    if (Parser.Parse()) {
        Parser.Var->Constant = Constant;
        Parser.Var->Visibility = VisKind;
        return AST->addGlobalVar(Parser.Var);
    }

    return false;
}

bool Parser::ParseFunctionDecl(VisibilityKind &VisKind, bool Constant, TypeDecl *TyDecl,
                               IdentifierInfo *Id, SourceLocation &IdLoc) {
    const StringRef &IdName = Id->getName();
    FunctionParser Parser(this, TyDecl, IdName, IdLoc);
    if (Parser.Parse()) {
        Parser.Function->Constant = Constant;
        Parser.Function->Visibility = VisKind;
        return AST->addFunction(Parser.Function);
    }

    return false;
}

bool Parser::ParseClassDecl(VisibilityKind &VisKind, bool &Constant) {
    ClassParser Parser(this);
    if (Parser.Parse()) {
        Parser.Class->Constant = Constant;
        Parser.Class->Visibility = VisKind;
        return AST->addClass(Parser.Class);
    }

    return false;
}

ASTNode *Parser::getAST() {
    return AST;
}

VarDecl* Parser::ParseVarDecl() {
    // Var Constant
    bool Constant = false;
    ParseScopes(Constant);

    // Var Type
    const TypeDecl *TyDecl = ParseType();

    // Var Name
    const StringRef Name = Tok.getIdentifierInfo()->getName();
    const SourceLocation IdLoc = Tok.getLocation();
    ConsumeToken();
    VarDecl *Var = new VarDecl(IdLoc, TyDecl, Name);
    Var->Constant = Constant;

    // Parsing =
    if (Tok.is(tok::equal)) {
        ConsumeToken();

        Expr* Ex = ParseExpr();
        if (Ex) {
            Var->Expression = Ex;
        }
    }

    return Var;
}

bool Parser::ParseStmt(Stmt *CurrentStmt, bool isBody) {
    if (Tok.is(tok::l_brace)) { // Open Brace
        Stmt *InnerStmt = (isBody) ? CurrentStmt : new Stmt(Tok.getLocation(), CurrentStmt->Vars);
        ConsumeBrace();
        // Enter in Sub-Statement
        if (ParseStmt(InnerStmt)) {
            CurrentStmt->Instructions.push_back(InnerStmt);
            return isBody || ParseStmt(CurrentStmt); // Continue with Current Statement
        }
    } else if (Tok.isAnyIdentifier()) {

    } else if (Tok.is(tok::kw_return)) {
        SourceLocation RetLoc = ConsumeToken();
        Expr *Exp = ParseExpr();
        if (Exp) {
            ReturnDecl *Return = new ReturnDecl(RetLoc, Exp);
            CurrentStmt->Return = Return;
            CurrentStmt->Instructions.push_back(Return);
            return ParseStmt(CurrentStmt);
        }
    } else if (Tok.is(tok::r_brace)) { // Close Brace
        ConsumeBrace();
        if (isBraceBalanced()) {
            // The Good Exit
            return true;
        } else {
            return ParseStmt(CurrentStmt);
        }
    }

    Diag(Tok.getLocation(), diag::err_parse_stmt);
    return false;
}

TypeDecl* Parser::ParseType() {
    TypeDecl *TyDecl = NULL;
    SourceLocation TypeLoc = Tok.getLocation();
    if (Tok.isOneOf(tok::kw_int, tok::kw_bool, tok::kw_float, tok::kw_void)) {
        switch (Tok.getKind()) {
            case tok::kw_bool:
                TyDecl = new BoolTypeDecl(TypeLoc);
                break;
            case tok::kw_int:
                TyDecl = new IntTypeDecl(TypeLoc);
                break;
            case tok::kw_float:
                TyDecl = new FloatTypeDecl(TypeLoc);
                break;
            case tok::kw_void:
                TyDecl = new VoidTypeDecl(TypeLoc);
                break;
            default:
                assert("Type Error");
        }
        ConsumeToken();
    } else {
        StringRef Name = Tok.getName();
        if (Name.empty()) {
            Diag(TypeLoc, diag::err_type_undefined);
            return TyDecl;
        }
        // TODO add class type
        ConsumeToken();
    }

    return TyDecl;
}

Expr* Parser::ParseExpr() {
    if (Tok.is(tok::numeric_constant)) {
        const StringRef V = StringRef(Tok.getLiteralData(), Tok.getLength());
        const SourceLocation &Loc = ConsumeToken();
        return new ValueExpr(Loc, V);
    } else if (Tok.isOneOf(tok::kw_true, tok::kw_false)) {
        StringRef B;
        switch (Tok.getKind()) {
            case tok::kw_true:
                B = "true";
                break;
            case tok::kw_false:
                B = "false";
                break;
            default:
                assert("Bool value not accepted");
        }
        const SourceLocation &Loc = ConsumeToken();
        return new ValueExpr(Loc, B);
    } else if (Tok.isAnyIdentifier()) {
        IdentifierInfo *OtherVar = Tok.getIdentifierInfo();
        const SourceLocation &Loc = ConsumeToken();
    }
}
