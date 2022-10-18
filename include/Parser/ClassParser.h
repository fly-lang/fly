//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/ClassParser.h - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CLASSPARSER_H
#define FLY_CLASSPARSER_H

namespace fly {

    class ASTClass;
    class ASTClassScopes;
    class ASTClassVar;
    class ASTClassFunction;
    class Parser;

    class ClassParser {

        friend class Parser;

        Parser *P;

        ASTClass *Class = nullptr;

        bool Success = true;

        ClassParser(Parser *P, ASTTopScopes *Scopes);

    public:

        static ASTClass *Parse(Parser *P, ASTTopScopes *Scopes);

        ASTClassScopes *ParseScopes();

        bool isField();

        bool ParseField(ASTClassScopes *Scopes, ASTType *Type);

        bool isMethod();

        bool ParseMethod(ASTClassScopes *Scopes, ASTType *Type);
    };
}

#endif //FLY_CLASSPARSER_H
