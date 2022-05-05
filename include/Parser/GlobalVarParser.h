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

    public:

        static ASTGlobalVar *Parse(Parser *P, ASTType *Type, VisibilityKind &Visibility, bool &Constant);
    };

}

#endif //FLY_GLOBALVARPARSER_H
