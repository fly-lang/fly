//===-------------------------------------------------------------------------------------------------------------===//
// include/Sym/SymComment.h - Sybolic Table for ASTComment
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_COMMENT_H
#define FLY_SYM_COMMENT_H


namespace fly {

    class ASTComment;

    class SymComment {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

    	ASTComment *AST;

        explicit SymComment(ASTComment *AST);

    public:

    	ASTComment *getAST() const;

    };

}

#endif //FLY_SYM_VAR_H
