//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/GlobalVarParser.h - Global Var Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_GLOBALVARPARSER_H
#define FLY_GLOBALVARPARSER_H

#include "Parser.h"
#include "Lex/Token.h"
#include "AST/GlobalVarDecl.h"

namespace fly {

    class Parser;

    class GlobalVarParser {

        friend Parser;

        Parser *P;
        TypeBase *TyDecl;
        const StringRef Name;
        SourceLocation &Location;
        GlobalVarDecl *Var = NULL;
        Expr *Val = NULL;

    public:
        GlobalVarParser(Parser *P, TypeBase *TyDecl, const StringRef &VarName,
                        SourceLocation &VarNameLoc);

        bool Parse();

        GlobalVarDecl *getVar() const;

        Expr *getVal() const;
    };
}

#endif //FLY_GLOBALVARPARSER_H
