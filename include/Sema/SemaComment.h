//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaComment.h - comment semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_COMMENT_H
#define FLY_SEMA_COMMENT_H

#include <string>


namespace fly {

    class ASTComment;

    class SemaComment {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

    	ASTComment &AST;

        explicit SemaComment(ASTComment &AST);

    public:

        ~SemaComment() = default;

    	ASTComment &getAST() const;

    	std::string str() const;

    };

}

#endif //FLY_SEMA_COMMENT_H
