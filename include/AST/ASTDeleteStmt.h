//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTDelete.h - Delete an Instance header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_DELETESTMT_H
#define FLY_AST_DELETESTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTBlockStmt;
    class ASTVarRef;

/**
 * Delete Stmt
 */
    class ASTDeleteStmt : public ASTStmt {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTVarRef *VarRef = nullptr;

    public:
        ASTDeleteStmt(const SourceLocation &Loc, ASTVarRef *VarRef);

        ASTVarRef *getVarRef();

        std::string str() const override;
    };
}
#endif //FLY_AST_DELETESTMT_H