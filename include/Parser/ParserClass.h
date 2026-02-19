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
    class ASTAttribute;
    class ASTMethod;
    class ASTParam;
    class Parser;

    class ParserClass {

        friend class Parser;
        friend class ParserMethod;

        Parser *P;

        ASTClass *Class = nullptr;

        unsigned short BraceCount = 0;

        ParserClass(Parser *P, llvm::SmallVector<ASTModifier *, 8> &Modifiers);

    public:

        static ASTClass *Parse(Parser *P, llvm::SmallVector<ASTModifier *, 8> &Modifiers);

        ASTAttribute *ParseAttribute(llvm::SmallVector<ASTModifier *, 8> &Modifiers, ASTType *TypeRef, const SourceLocation &Loc, llvm::StringRef Name);

        ASTMethod *ParseMethod(llvm::SmallVector<ASTModifier *, 8> &Modifiers, const SourceLocation &Loc, llvm::StringRef Name);
    };
}

#endif //FLY_PARSERCLASS_H
