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
    class ASTFunction;
    class ASTVar;
    class Parser;

    class ParserClass {

        friend class Parser;
        friend class ParserMethod;

        Parser *P;

        ASTClass *Class = nullptr;

        unsigned short BraceCount = 0;

        ParserClass(Parser *P, llvm::SmallVector<ASTScope *, 8> &Scopes);

    public:

        static ASTClass *Parse(Parser *P, llvm::SmallVector<ASTScope *, 8> &Scopes);

        ASTVar *ParseAttribute(llvm::SmallVector<ASTScope *, 8> &Scopes, ASTType *Type, const SourceLocation &Loc, llvm::StringRef Name);

        ASTFunction *ParseMethod(llvm::SmallVector<ASTScope *, 8> &Scopes, ASTType *Type, const SourceLocation &Loc, llvm::StringRef Name);
    };
}

#endif //FLY_PARSERCLASS_H
