//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTLoopInStmt.h - AST Loop In Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_LOOPINSTMT_H
#define FLY_AST_LOOPINSTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTRef;

    class ASTLoopInStmt : public ASTStmt {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        ASTRef *VarRef = nullptr;

        ASTBlockStmt *Block = nullptr;

        explicit ASTLoopInStmt(const SourceLocation &Loc);

    public:

        ASTRef *getVarRef() const;

        ASTBlockStmt *getBlock() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_LOOPINSTMT_H
