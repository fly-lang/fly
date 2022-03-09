//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTResolver.h - Resolve AST by traversing all ASTNode
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTRESOLVER_H
#define FLY_ASTRESOLVER_H

#include "llvm/ADT/StringMap.h"
#include <vector>

namespace fly {

    class ASTContext;
    class ASTNameSpace;
    class ASTNode;
    class ASTUnrefGlobalVar;
    class ASTGlobalVar;
    class ASTUnrefCall;
    class ASTFuncCall;
    class ASTType;
    class ASTExpr;
    class ASTLocalVar;
    class ASTBlock;
    class ASTVarRef;

    class ASTResolver {

    public:
        static bool Resolve(ASTNameSpace *NameSpace);

        static bool Resolve(ASTNode *Node);

        static bool ResolveImports(ASTNameSpace *NameSpace);

        static bool ResolveGlobalVars(ASTNode *Node);

        static bool ResolveGlobalVars(ASTNameSpace *NameSpace);

        static bool ResolveFuncCalls(ASTNode *Node);

        static bool ResolveFuncCalls(ASTNameSpace *NameSpace);

        static ASTType *ResolveExprType(ASTExpr *Expr);

        static ASTLocalVar *FindVarDecl(const ASTBlock *Block, ASTVarRef *VarRef);

        static bool ResolveVarRef(const ASTBlock *Block, ASTVarRef *VarRef);

        static bool ResolveExpr(const ASTBlock *Block, const ASTExpr *Expr);
    };
}

#endif //FLY_ASTRESOLVER_H
