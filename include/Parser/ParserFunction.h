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

        explicit ParserFunction(Parser *P);

        static ASTFunction *Parse(Parser *P, llvm::SmallVector<ASTScope *, 8> Scopes, ASTTypeRef *Type);

        ASTBlockStmt *ParseBody(ASTFunction *F);

        llvm::SmallVector<ASTVar *, 8> ParseParams();

    private:

        ASTVar *ParseParam();

    };
}


#endif //FLY_PARSERFUNCTION_H
