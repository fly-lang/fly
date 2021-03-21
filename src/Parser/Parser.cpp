//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/Parser.cpp - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include <string>

using namespace fly;
using namespace std;

Parser::Parser(const string &fileName, Lexer &L, DiagnosticsEngine &Diags) :
        Lex(L), Diags(Diags) {

    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();

    // Parse Package on first
    ParsePackageDecl(fileName);

}


DiagnosticBuilder Parser::Diag(SourceLocation Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

DiagnosticBuilder Parser::Diag(const Token &Tok, unsigned DiagID) {
    return Diag(Tok.getLocation(), DiagID);
}

ASTContext& Parser::getASTContext() {
    return *Context;
}

bool Parser::ParsePackageDecl(const string& fileName) {
    if (Tok.is(tok::kw_package)) {
        SourceLocation StartLoc = Tok.getLocation();
        SourceLocation PackageNameLoc = ConsumeToken();
        if (Tok.isLiteral()) {
            const PackageDecl &Package = PackageDecl(Tok.getLiteralData());
            Context = new ASTContext(fileName, Package);
            return true;
        }

        Diag(Tok, diag::err_package_undefined);

    } else {
        Diag(Tok,diag::err_package_missing); // Unexpected diagnostic outside source file processing
    }
    return false;
}


