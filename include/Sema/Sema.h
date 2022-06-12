//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Sema.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_H
#define FLY_SEMA_H

#include "AST/ASTFunction.h"
#include "AST/ASTImport.h"
#include "AST/ASTFunctionCall.h"

namespace fly {

    class SemaBuilder;
    class SemaResolver;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;
    class ASTContext;
    class ASTNameSpace;
    class ASTNode;
    class ASTBlock;
    class ASTLocalVar;
    class ASTVarRef;
    class ASTExpr;
    class ASTType;
    class ASTIfBlock;
    class ASTSwitchBlock;
    class ASTWhileBlock;
    class ASTForBlock;


    class Sema {

        friend class SemaBuilder;
        friend class SemaResolver;

        DiagnosticsEngine &Diags;

        ASTContext *Context;

        SemaResolver *Resolver;

        Sema(DiagnosticsEngine &Diags);

    public:

        static SemaBuilder* Builder(DiagnosticsEngine &Diags);

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

        DiagnosticBuilder Diag(unsigned DiagID) const;

        bool CheckDuplicatedLocalVars(ASTBlock *Block, ASTLocalVar *LocalVar);

        bool CheckUndef(ASTBlock *Block, ASTVarRef *VarRef);

        static bool CheckOnCloseBlock(ASTBlock *Block); // TODO

        bool CheckImport(ASTNode *Node, ASTImport *Import);

        bool CheckExpr(ASTExpr *Expr);

        bool isEquals(ASTParam *Param1, ASTParam *Param2);

        bool isTypeDerivate(ASTType *T1, ASTType *T2);

        bool CheckValueType(ASTValueExpr *ValueExpr, ASTType *Type);

        bool CheckIfBlock(ASTIfBlock *Block);

        bool CheckSwitchBlock(ASTSwitchBlock *Block);

        bool CheckWhileBlock(ASTWhileBlock *Block);

        bool CheckForBlock(ASTForBlock *Block);
    };

}  // end namespace fly

#endif