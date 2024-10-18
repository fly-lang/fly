//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/ClassParser.h - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_PARSERCLASS_H
#define FLY_PARSERCLASS_H

namespace fly {

    class ASTClass;
    class ASTClassAttribute;
    class ASTClassMethod;
    class Parser;

    class ParserClass {

        friend class Parser;

        Parser *P;

        ASTClass *Class = nullptr;

        unsigned short BraceCount = 0;

        ParserClass(Parser *P, ASTComment *Comment, SmallVector<ASTScope *, 8> &Scopes);

    public:

        static ASTClass *Parse(Parser *P, ASTComment *Comment, SmallVector<ASTScope *, 8> &Scopes);

        ASTClassAttribute *ParseAttribute(SmallVector<ASTScope *, 8> &Scopes, ASTType *Type, const SourceLocation &Loc, llvm::StringRef Name);

        ASTClassMethod *ParseMethod(SmallVector<ASTScope *, 8> &Scopes, ASTType *Type, const SourceLocation &Loc, llvm::StringRef Name);
    };
}

#endif //FLY_PARSERCLASS_H
