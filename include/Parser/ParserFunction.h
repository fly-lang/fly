//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/FunctionParser.h - Function Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_PARSERFUNCTION_H
#define FLY_PARSERFUNCTION_H

#include "llvm/ADT/StringRef.h"

namespace fly {

    class Parser;
    class ASTFunction;
    class ASTFunctionBase;
    class ASTParam;
    class ASTType;
    class ASTBlockStmt;
    class SourceLocation;

    class ParserFunction {

        friend class Parser;

        static ASTParam *ParseParam(Parser *P);

    public:

        static llvm::SmallVector<ASTParam *, 8> ParseParams(Parser *P);

        static ASTBlockStmt *ParseBody(Parser *P);

    };
}


#endif //FLY_PARSERFUNCTION_H
