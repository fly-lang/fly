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
#include "AST/ASTGlobalVar.h"

namespace fly {

    class Parser;

    class GlobalVarParser {

        friend Parser;

        Parser *P;
        ASTType *Type;
        ASTGlobalVar *AST = nullptr;

    public:
        GlobalVarParser(Parser *P, ASTType *Type);

        bool Parse();

        ASTGlobalVar *getAST() const;
    };

}

#endif //FLY_GLOBALVARPARSER_H
