//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/ParserFunction.h - function declaration and call parser
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
    class ASTParam;
    class ASTType;
    class ASTBlockStmt;
    class ASTModifier;
    class ASTComment;
    class SourceLocation;

    class ParserFunction {

        friend class Parser;

        Parser *P;

    public:

        static llvm::SmallVector<ASTParam *, 8> ParseParams(Parser *P);

        static ASTBlockStmt *ParseBody(Parser *P, ASTFunction *F);

    private:

        static ASTParam *ParseParam(Parser *P);

    };
}


#endif //FLY_PARSERFUNCTION_H
