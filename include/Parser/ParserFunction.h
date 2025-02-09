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
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class Parser;
    class ASTFunction;
    class ASTFunction;
    class ASTVar;
    class ASTTypeRef;
    class ASTBlockStmt;
    class ASTScope;
    class ASTComment;
    class SourceLocation;

    class ParserFunction {

        friend class Parser;

        Parser *P;

    public:

        static llvm::SmallVector<ASTVar *, 8> ParseParams(Parser *P);

        static ASTBlockStmt *ParseBody(Parser *P, ASTFunction *F);

    private:

        static ASTVar *ParseParam(Parser *P);

    };
}


#endif //FLY_PARSERFUNCTION_H
