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
#include "AST/ASTType.h"


namespace fly {

    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;
    class CodeGen;
    class ASTContext;
    class ASTNameSpace;
    class ASTNode;
    class ASTNodeBase;
    class ASTImport;
    class ASTClass;
    class ASTFunction;
    class ASTFunctionCall;
    class ASTBlock;
    class ASTIfBlock;
    class ASTElsifBlock;
    class ASTElseBlock;
    class ASTSwitchBlock;
    class ASTSwitchCaseBlock;
    class ASTSwitchDefaultBlock;
    class ASTWhileBlock;
    class ASTForBlock;
    class ASTExprStmt;
    class ASTVarAssign;
    class ASTLocalVar;
    class ASTVarRef;
    class ASTExpr;
    class ASTEmptyExpr;
    class ASTType;
    class ASTBoolValue;
    class ASTIntegerValue;
    class ASTFloatingValue;
    class ASTNullValue;
    class ASTBoolValue;
    class ASTArrayValue;
    class ASTIntegerValue;
    class ASTFloatingValue;


    class SemaBuilder {

        Sema &S;

    public:

        SemaBuilder(Sema &S);

        bool Build();

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

        DiagnosticBuilder Diag(unsigned DiagID) const;

        ASTNode *CreateHeaderNode(const std::string &Name, std::string &NameSpace);

        ASTNode *CreateNode(const std::string &Name, std::string &NameSpace);

        // Create Top Definitions
        ASTImport *CreateImport(const SourceLocation &NameLoc, StringRef Name);
        ASTImport *CreateImport(const SourceLocation &NameLoc, StringRef Name,
                                const SourceLocation &AliasLoc, StringRef Alias);
        ASTGlobalVar *CreateGlobalVar(ASTNode *Node, const SourceLocation Loc, ASTType *Type, const std::string &Name,
                                       VisibilityKind &Visibility, bool &Constant);
        ASTFunction *CreateFunction(ASTNode *Node, const SourceLocation Loc, ASTType *Type, const std::string &Name,
                                    VisibilityKind &Visibility);
        ASTClass *CreateClass(ASTNode *Node, const SourceLocation Loc, const std::string &Name, VisibilityKind &Visibility,
                              bool &Constant);

        // Create Function Call
        ASTFunctionCall *CreateFunctionCall(const SourceLocation &Loc, std::string &Name, std::string &NameSpace);
        ASTFunctionCall *CreateFunctionCall(ASTFunction *Function);

        // Create Types
        ASTType *CreateBoolType(const SourceLocation &Loc);
        ASTType *CreateByteType(const SourceLocation &Loc);
        ASTType *CreateUShortType(const SourceLocation &Loc);
        ASTType *CreateShortType(const SourceLocation &Loc);
        ASTType *CreateUIntType(const SourceLocation &Loc);
        ASTType *CreateIntType(const SourceLocation &Loc);
        ASTType *CreateULongType(const SourceLocation &Loc);
        ASTType *CreateLongType(const SourceLocation &Loc);
        ASTType *CreateFloatType(const SourceLocation &Loc);
        ASTType *CreateDoubleType(const SourceLocation &Loc);
        ASTType *CreateVoidType(const SourceLocation &Loc);
        ASTArrayType *CreateArrayType(const SourceLocation &Loc, ASTType *Type, ASTExpr *Size);
        ASTClassType *CreateClassType(const SourceLocation &Loc, StringRef Name, StringRef NameSpace);
        ASTClassType *CreateClassType(ASTClass *Class);

        // Create Values
        ASTNullValue *CreateNullValue(const SourceLocation &Loc);
        ASTBoolValue *CreateBoolValue(const SourceLocation &Loc, bool Val);
        ASTIntegerValue *CreateIntegerValue(const SourceLocation &Loc, uint64_t Val, bool Negative = false);
        ASTIntegerValue *CreateCharValue(const SourceLocation &Loc, char Val);
        ASTFloatingValue *CreateFloatingValue(const SourceLocation &Loc, double Val);
        ASTValue *CreateValue(const SourceLocation &Loc, std::string &Val);
        ASTArrayValue *CreateArrayValue(const SourceLocation &Loc);
        ASTValue *CreateDefaultValue(ASTType *Type);

        ASTLocalVar *CreateLocalVar(const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant,
                                    ASTExpr *Expr = nullptr);

        ASTParam *CreateParam(const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant);

        ASTExprStmt *CreateExprStmt(const SourceLocation &Loc, ASTExpr *Expr);

        ASTEmptyExpr *CreateExpr(const SourceLocation &Loc);

        ASTValueExpr *CreateExpr(ASTValue *Value);

        ASTFuncCallExpr *CreateExpr(ASTFunctionCall *Call);

        ASTVarRefExpr *CreateExpr(ASTVarRef *VarRef);

        ASTVarAssign *CreateVarAssign(const SourceLocation &Loc, ASTVarRef * VarRef, ASTExpr *Expr);

        ASTVarRef *CreateVarRef(const SourceLocation &Loc, StringRef Name, StringRef NameSpace);

        ASTVarRef *CreateVarRef(ASTLocalVar *LocalVar);

        ASTVarRef *CreateVarRef(ASTGlobalVar *GlobalVar);

        ASTBlock* CreateBlock(const SourceLocation &Loc);

        ASTIfBlock *CreateIfBlock(const SourceLocation &Loc, ASTExpr *Condition);

        ASTElsifBlock *CreateElsifBlock(const SourceLocation &Loc, ASTExpr *Condition);

        ASTElseBlock *CreateElseBlock(const SourceLocation &Loc);

        ASTSwitchBlock *CreateSwitchBlock(const SourceLocation &Loc, ASTExpr *Expr);

        ASTSwitchCaseBlock *CreateSwitchCaseBlock(const SourceLocation &Loc, ASTExpr *Condition);

        ASTSwitchDefaultBlock *CreateSwitchDefaultBlock(const SourceLocation &Loc);

        ASTWhileBlock *CreateWhileBlock(const SourceLocation &Loc, ASTExpr *Condition);

        ASTForBlock *CreateForBlock(const SourceLocation &Loc, ASTExpr *Condition, ASTBlock *PostBlock,
                                    ASTBlock *LoopBlock);

        ASTNameSpace *AddNameSpace(const std::string &Name, bool ExternLib = false);

        bool AddNode(ASTNode *Node);

        bool AddImport(ASTNode *Node, ASTImport *Import);

        bool AddGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar, ASTExpr *Expr);

        bool AddFunction(ASTNode *Node, ASTFunction *Function);

        bool AddComment(ASTTopDef *Top, std::string &Comment);

        bool AddExternalGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar);

        bool AddExternalFunction(ASTNode *Node, ASTFunction *Call);

        bool AddFunctionParam(ASTFunction *Function, ASTParam *Param);

        void AddVarArgs(ASTFunction *Function, ASTParam *Param);

        bool AddFunctionCall(ASTNodeBase *Base, ASTFunctionCall *Call);

        bool AddCallArg(ASTFunctionCall *Call, ASTExpr *Expr);

        bool AddClass(ASTNode *Node, ASTClass *Class);

        bool AddUnrefGlobalVar(ASTNode *Node, ASTVarRef *VarRef);

        bool AddUnrefCall(ASTNode *Node, ASTFunctionCall *Call);

        bool setVarExpr(ASTVar *Var, ASTExpr *Expr);

        bool AddStmt(ASTBlock *Block, ASTStmt *Stmt);

        bool AddLocalVar(ASTBlock *Block, ASTLocalVar *LocalVar, bool PushToContent = true);

        bool AddVarAssign(ASTBlock *Block, ASTVarAssign *VarAssign);

        bool RemoveUndefVar(ASTBlock *Block, ASTVarRef *VarRef);

        bool AddReturn(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr);

        bool AddBreak(ASTBlock *Block, const SourceLocation &Loc);

        bool AddContinue(ASTBlock *Block, const SourceLocation &Loc);

        bool AddIfBlock(ASTBlock *Block, ASTIfBlock *IfBlock);

        bool AddElsifBlock(ASTIfBlock *IfBlock, ASTElsifBlock *ElsifBlock);

        bool AddElseBlock(ASTIfBlock *IfBlock, ASTElseBlock *ElseBlock);

        bool AddSwitchBlock(ASTBlock *Parent, ASTSwitchBlock *Block);

        bool AddSwitchCaseBlock(ASTSwitchBlock *SwitchBlock, ASTSwitchCaseBlock *CaseBlock);

        bool setSwitchDefaultBlock(ASTSwitchBlock *SwitchBlock, ASTSwitchDefaultBlock *DefaultBlock);

        bool AddWhileBlock(ASTBlock *Block, ASTWhileBlock *WhileBlock);

        bool AddForBlock(ASTBlock *Block, ASTForBlock *ForBlock);

        bool AddArrayValue(ASTArrayValue *Array, ASTValue *Value);

        ASTStmt *getLastStmt(ASTBlock *Block);

        bool OnCloseBlock(ASTBlock *Block);
    };

}  // end namespace fly

#endif