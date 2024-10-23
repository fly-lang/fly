//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/ParserMethod.h - Method Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_PARSERMETHOD_H
#define FLY_PARSERMETHOD_H

#include "ParserFunction.h"
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class Parser;
    class ASTParam;
    class ASTType;
    class ASTBlockStmt;
    class SourceLocation;
    class ASTScope;
    class ASTComment;
    class ASTClassMethod;
    class ParserClass;

    class ParserMethod : ParserFunction {

        friend class Parser;

        ParserClass *PC;

    public:

        explicit ParserMethod(ParserClass *PC);

        static ASTClassMethod *Parse(ParserClass *PC, llvm::SmallVector<ASTScope *, 8> Scopes, ASTType *Type,
                                  const SourceLocation &Loc, llvm::StringRef Name);

    };
}


#endif //FLY_PARSERMETHOD_H
