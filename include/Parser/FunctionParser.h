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
#include "AST/ASTFunc.h"

namespace fly {

    class Parser;

    class FunctionParser {

        friend class Parser;

        Parser *P;
        ASTFunc *AST = nullptr;
        ASTFuncCall *Call = nullptr;

        FunctionParser(Parser *P);

        bool ParseFunction(ASTType *Type);

        bool ParseFunctionBody();

        bool ParseFunctionParams();

        bool ParseFunctionParam();

        bool ParseCall(ASTBlock *Block, SourceLocation &Loc, llvm::StringRef Name, llvm::StringRef NameSpace = "");

        bool ParseCallArgs(ASTBlock *Block);

        bool ParseCallArg(ASTBlock *Block);
    };
}


#endif //FLY_FUNCTIONPARSER_H
