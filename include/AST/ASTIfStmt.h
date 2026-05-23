//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIfStmt.h - AST if statement header
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

        friend class ASTBuilderIfStmt;

        // The list of Elseif Blocks
        llvm::SmallVector<ASTRuleStmt *, 8> Elsif;

        // The Else Block
        ASTStmt *Else = nullptr;

        explicit ASTIfStmt(const SourceLocation &Loc);

    public:

        void accept(ASTVisitor& Visitor) override;

        llvm::SmallVector<ASTRuleStmt *, 8> getElsif();

        ASTStmt *getElse();

        std::string str() const override;
    };

}


#endif //FLY_AST_IFSTMT_H
