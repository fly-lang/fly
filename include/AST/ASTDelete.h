//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTDelete.h - Delete an Instance
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTDELETE_H
#define FLY_ASTDELETE_H

#include "ASTStmt.h"

namespace fly {

    class ASTBlock;
    class ASTVarRef;

/**
 * Delete Stmt
 */
    class ASTDelete : public ASTStmt {

        ASTVarRef *VarRef = nullptr;

    public:
        ASTDelete(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *VarRef);

        ASTVarRef *getVarRef();

        std::string str() const override;
    };
}
#endif //FLY_ASTDELETE_H