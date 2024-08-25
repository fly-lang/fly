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

        friend class SemaBuilder;
        friend class SemaBuilderStmt;
        friend class SemaBuilderIfStmt;
        friend class SemaBuilderSwitchStmt;
        friend class SemaBuilderLoopStmt;
        friend class SemaResolver;
        friend class SemaValidator;

        // List of Statements of the Block
        llvm::SmallVector<ASTStmt *, 8> Content;

        // Contains all vars declared in this Block
        llvm::StringMap<ASTLocalVar *> LocalVars;

    protected:

        explicit ASTBlockStmt(const SourceLocation &Loc);

    public:

        const llvm::SmallVector<ASTStmt *, 8> &getContent() const;

        bool isEmpty() const;

        void Clear();

        const llvm::StringMap<ASTLocalVar *> &getLocalVars() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_BLOCKSTMT_H
