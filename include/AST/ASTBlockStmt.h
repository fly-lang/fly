//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBlockStmt.h - AST Block Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BLOCKSTMT_H
#define FLY_AST_BLOCKSTMT_H

#include "ASTStmt.h"

#include "llvm/ADT/StringMap.h"

namespace fly {

    class ASTLocalVar;

    /**
     * AST Block
     */
    class ASTBlockStmt : public ASTStmt {

        friend class ASTBuilder;

    protected:

        // List of Statements of the Block
        llvm::SmallVector<ASTStmt *, 8> Content;

        explicit ASTBlockStmt(const SourceLocation &Loc);

    public:

        ~ASTBlockStmt() override;

        void accept(ASTVisitor& Visitor) override;

        llvm::SmallVector<ASTStmt *, 8> &getContent();

        // Add a statement to the block and set its parent/function
        void addContent(ASTStmt *Stmt);

        bool isEmpty() const;

        void Clear();

        std::string str() const override;
    };
}

#endif //FLY_AST_BLOCKSTMT_H
