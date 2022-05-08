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
    class ASTFunction;
    class ASTFunctionCall;
    class ASTCallArg;
    class ASTBlock;
    class ASTIfBlock;
    class ASTElsifBlock;
    class ASTElseBlock;
    class ASTSwitchBlock;
    class ASTWhileBlock;
    class ASTForBlock;
    class ASTExprStmt;
    class ASTVarAssign;
    class ASTLocalVar;
    class ASTVarRef;
    class ASTExpr;
    class ASTType;
    class ASTBoolValue;
    class ASTIntegerValue;
    class ASTFloatingValue;

    class SemaBuilder {

        Sema &S;

    public:

        SemaBuilder(Sema &S);

        bool Build();

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

        DiagnosticBuilder Diag(unsigned DiagID) const;

        ASTNameSpace *AddNameSpace(const std::string &Name, bool ExternLib = false);

        ASTNode *CreateHeaderNode(const std::string &Name, std::string &NameSpace);

        ASTNode *CreateNode(const std::string &Name, const std::string &NameSpace);

        bool AddNode(ASTNode *Node);

        bool AddImport(ASTNode *Node, ASTImport *Import);

        bool AddExternalGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar);

        bool AddExternalFunction(ASTNode *Node, ASTFunction *Call);

        bool AddComment(ASTTopDecl *Top, std::string Comment);

        ASTGlobalVar * CreateGlobalVar(ASTNode *Node, SourceLocation Loc, ASTType *Type, const std::string &Name,
                                       VisibilityKind &Visibility, bool &Constant);

        bool AddGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar, ASTValueExpr *Expr);

        bool AddFunction(ASTNode *Node, ASTFunction *Function);

        ASTParam *CreateParam(const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant);

        bool AddFunctionParam(ASTFunction *Function, ASTParam *Param);

        void AddVarArgs(ASTFunction *Function, ASTParam *Para);

        ASTFunctionCall *CreateFunctionCall(SourceLocation &Location, std::string &Name, std::string &NameSpace);

        ASTFunctionCall *CreateFunctionCall(ASTFunction *Function);

        bool AddFunctionCall(ASTNodeBase *Base, ASTFunctionCall *Call);

        bool AddFunctionCall(ASTBlock *Block, ASTFunctionCall *Call);

        ASTCallArg *CreateCallArg(ASTType *Type);

        ASTCallArg *CreateCallArg(ASTExpr *Expr);

        bool AddCallArg(ASTFunctionCall *Call, ASTCallArg *Arg);

        bool AddClass(ASTNode *Node, ASTClass *Class);

        bool AddUnrefCall(ASTNode *Node, ASTFunctionCall *Call);

        bool AddUnrefGlobalVar(ASTNode *Node, ASTVarRef *VarRef);

        bool setVarExpr(ASTVar *Var, ASTValueExpr *ValueExpr);

        bool AddExprStmt(ASTBlock *Block, ASTExprStmt *ExprStmt);

        ASTExpr *CreateExpr(const SourceLocation &Loc, ASTValue *Value);

        ASTExpr *CreateExpr(const SourceLocation &Loc, ASTFunctionCall *Call);

        ASTExpr *CreateExpr(const SourceLocation &Loc, ASTVarRef *VarRef);

        ASTBoolValue *CreateValue(const SourceLocation &Loc, bool Val);

        ASTIntegerValue *CreateValue(const SourceLocation &Loc, int Val);

        ASTFloatingValue *CreateValue(const SourceLocation &Loc, std::string Val);

        ASTLocalVar *CreateLocalVar(ASTBlock *Block, const SourceLocation &Loc, ASTType *Type, const std::string &Name,
                                    bool Constant);

        bool AddLocalVar(ASTBlock *Block, ASTLocalVar *LocalVar, ASTExpr *Expr);

        ASTVarRef *CreateVarRef(ASTLocalVar *LocalVar);

        ASTVarRef *CreateVarRef(ASTGlobalVar *GlobalVar);

        bool AddVarAssign(ASTBlock *Block, ASTVarAssign *VarAssign);

        bool RemoveUndefVar(ASTBlock *Block, ASTVarRef *VarRef);

        bool AddReturn(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr);

        bool AddBreak(ASTBlock *Block, const SourceLocation &Loc);

        bool AddContinue(ASTBlock *Block, const SourceLocation &Loc);

        ASTBlock* CreateBlock(const SourceLocation &Loc);

        ASTIfBlock* AddIfBlock(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr);

        ASTElsifBlock *AddElsifBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc, ASTExpr *Expr);

        ASTElseBlock *AddElseBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc);

        ASTSwitchBlock* AddSwitchBlock(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr);

        ASTWhileBlock* AddWhileBlock(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr);

        ASTForBlock *CreateForBlock(const SourceLocation &Loc, ASTExpr *Condition, ASTBlock *PostBlock, ASTBlock *LoopBlock);

        bool AddForBlock(ASTBlock *Block, ASTForBlock *ForBlock);

        bool OnCloseBlock(ASTBlock *Block);

        ASTStmt *getLastStmt(ASTBlock *Block);
    };

}  // end namespace fly

#endif