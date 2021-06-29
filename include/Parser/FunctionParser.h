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
#include "AST/FuncDecl.h"

namespace fly {

    class Parser;

    class FunctionParser {

        friend class Parser;

        Parser *P;
        const llvm::StringRef &FuncName;
        SourceLocation &FuncNameLoc;
        FuncDecl *Function = NULL;
        FuncCall *Call = NULL;

        bool ParseBody();

        FunctionParser(Parser *P, const llvm::StringRef &FuncName, SourceLocation &FuncNameLoc);

        bool ParseCall(BlockStmt *CurrStmt, llvm::StringRef NameSpace = "");

        bool ParseDefinition(TypeBase *TyDecl);

        bool ParseParams();

        FuncParam* ParseParam();

        bool ParseArgs();

        bool ParseHeader(bool isStart = false, bool isRef = false);
    };
}


#endif //FLY_FUNCTIONPARSER_H
