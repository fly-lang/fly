//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIfBlock.h - AST If Block Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_IFSTMT_H
#define FLY_AST_IFSTMT_H

#include "ASTRuleStmt.h"

namespace fly {

    class ASTIfStmt : public ASTRuleStmt {

        friend class ASTBuilder;
        friend class SemaBuilderIfStmt;
        friend class SemaResolver;
        friend class SemaValidator;

        // The list of Elseif Blocks
        llvm::SmallVector<ASTRuleStmt *, 8> Elsif;

        // The Else Block
        ASTStmt *Else = nullptr;

        explicit ASTIfStmt(const SourceLocation &Loc);

    public:

        llvm::SmallVector<ASTRuleStmt *, 8> getElsif();

        ASTStmt *getElse();

        std::string str() const override;
    };

}


#endif //FLY_AST_IFSTMT_H
