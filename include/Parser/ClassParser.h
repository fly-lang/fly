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
    class ASTScopes;
    class ASTClassVar;
    class ASTClassFunction;
    class ASTScopes;
    class Parser;

    class ClassParser {

        friend class Parser;

        Parser *P;

        ASTClass *Class = nullptr;

        bool Success = true;

        unsigned short BraceCount = 0;

        ClassParser(Parser *P, ASTScopes *ClassScopes);

    public:

        static ASTClass *Parse(Parser *P, ASTScopes *ClassScopes);

        bool ParseField(ASTScopes *Scopes, ASTType *Type, const SourceLocation &Loc, llvm::StringRef Name);

        bool ParseMethod(ASTScopes *Scopes, ASTType *Type, const SourceLocation &Loc, llvm::StringRef Name);

        SourceLocation ConsumeBrace();
    };
}

#endif //FLY_CLASSPARSER_H
