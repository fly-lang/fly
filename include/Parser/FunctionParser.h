//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/FunctionParser.h - Function Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_FUNCTIONPARSER_H
#define FLY_FUNCTIONPARSER_H

#include "Parser.h"
#include "Lex/Token.h"
#include "AST/FunctionDecl.h"

namespace fly {

    class Parser;

    class FunctionParser {

        friend class Parser;

        Parser *P;
        TypeDecl *RetTyDecl;
        const StringRef &FuncName;
        SourceLocation &FuncNameLoc;
        FunctionDecl *Function;

        bool ParseParameters(bool isStart = false);

        bool ParseBody();

    public:
        FunctionParser(Parser *P, TypeDecl *RetTyDecl, const StringRef &FuncName,
                       SourceLocation &FuncNameLoc);

        bool Parse();

    };
}


#endif //FLY_FUNCTIONPARSER_H
