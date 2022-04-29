//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Sema.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_SEMA_H
#define FLY_SEMA_SEMA_H

namespace fly {

    class ASTNode;
    class ASTVarRef;
    class ASTExpr;
    class ASTBlock;

    class Sema {

        const ASTNode *AST;

    public:
        Sema(ASTNode *AST);

        static bool CheckUndefVar(ASTBlock *Block, ASTVarRef *VarRef);

        static bool CheckOnCloseBlock(ASTBlock *Block);
    };

}  // end namespace fly

#endif