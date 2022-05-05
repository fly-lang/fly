//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Sema.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BUILDER_H
#define FLY_SEMA_BUILDER_H

#include "Sema/Sema.h"
#include "AST/ASTClass.h"

namespace fly {

    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;
    class ASTContext;
    class ASTNameSpace;
    class ASTNode;
    class ASTNodeBase;
    class ASTImport;
    class ASTClass;
    class ASTBlock;
    class ASTIfBlock;
    class ASTSwitchBlock;
    class ASTWhileBlock;
    class ASTForBlock;
    class ASTExprStmt;
    class ASTVarAssign;
    class ASTLocalVar;
    class ASTVarRef;
    class ASTExpr;
    class ASTType;

    class SemaBuilder {

        Sema &S;

    public:

        SemaBuilder(Sema &S);

        bool Build();

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

        DiagnosticBuilder Diag(unsigned DiagID) const;

        bool AddImport(ASTNode *Node, ASTImport *Import);

        bool AddComment(ASTTopDecl *Top, std::string Comment);

        bool AddGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar);

        bool AddFunction(ASTNode *Node, ASTFunction *Function);

        bool AddFunctionParam(ASTFunction *Function, ASTParam *Param);

        ASTParam *AddParam(ASTFunction *Function, const SourceLocation &Loc, ASTType *Type, const std::string &Name,
                           bool Constant);

        void setVarArg(ASTFunction *Function, ASTParam* VarArg);

        bool AddFunctionCall(ASTNodeBase *Base, ASTFunctionCall *Call);

        ASTFunctionCall *CreateCall(ASTFunction *Function);

        bool AddClass(ASTNode *Node, ASTClass *Class);

        bool AddUnrefCall(ASTNode *Node, ASTFunctionCall *Call);

        bool AddUnrefGlobalVar(ASTNode *Node, ASTVarRef *VarRef);

        bool setVarExpr(ASTParam *Param, ASTValueExpr *ValueExpr);

        bool AddExprStmt(ASTBlock *Block, ASTExprStmt *ExprStmt);

        bool AddLocalVar(ASTBlock *Block, ASTLocalVar *LocalVar);

        bool AddVarAssign(ASTBlock *Block, ASTVarAssign *VarAssign);

        bool RemoveUndefVar(ASTBlock *Block, ASTVarRef *VarRef);

        bool AddCall(ASTBlock *Block, ASTFunctionCall *Invoke);

        bool AddReturn(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr);

        bool AddBreak(ASTBlock *Block, const SourceLocation &Loc);

        bool AddContinue(ASTBlock *Block, const SourceLocation &Loc);

        ASTIfBlock* AddIfBlock(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr);

        ASTSwitchBlock* AddSwitchBlock(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr);

        ASTWhileBlock* AddWhileBlock(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr);

        ASTForBlock* AddForBlock(ASTBlock *Block, const SourceLocation &Loc);

        void setCondition(ASTForBlock *ForBlock, ASTExpr *Expr);

        bool OnCloseBlock(ASTBlock *Block);
    };

}  // end namespace fly

#endif