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
    class ASTBreak;
    class ASTContinue;

    class SemaBuilder {

        Sema &S;

    public:

        SemaBuilder(Sema &S);

        bool Build();

        // Diagnostics Functions
        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;
        DiagnosticBuilder Diag(unsigned DiagID) const;

        // Create Node
        ASTNode *CreateNode(const std::string &Name, std::string &NameSpace);
        ASTNode *CreateHeaderNode(const std::string &Name, std::string &NameSpace);

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

        // Create Statements
        ASTFunctionCall *CreateFunctionCall(ASTStmt *Parent, const SourceLocation &Loc, std::string &Name, std::string &NameSpace);
        ASTArg *CreateArg(const SourceLocation &Loc);
        ASTParam *CreateParam(const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant);
        ASTLocalVar *CreateLocalVar(const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant);
        ASTVarAssign *CreateVarAssign(ASTVarRef *VarRef);
        ASTReturn *CreateReturn(const SourceLocation &Loc);
        ASTBreak *CreateBreak(const SourceLocation &Loc);
        ASTContinue *CreateContinue(const SourceLocation &Loc);
        ASTExprStmt *CreateExprStmt(const SourceLocation &Loc);

        // Create Var References
        ASTVarRef *CreateVarRef(const SourceLocation &Loc, StringRef Name, StringRef NameSpace);
        ASTVarRef *CreateVarRef(ASTLocalVar *LocalVar);
        ASTVarRef *CreateVarRef(ASTGlobalVar *GlobalVar);

        // Create Expressions
        ASTEmptyExpr *CreateExpr(const SourceLocation &Loc);
        ASTValueExpr *CreateExpr(ASTStmt *Stmt, ASTValue *Value);
        ASTFunctionCallExpr *CreateExpr(ASTStmt *Stmt, ASTFunctionCall *Call);
        ASTVarRefExpr *CreateExpr(ASTStmt *Stmt, ASTVarRef *VarRef);

        // Create Blocks structures
        ASTBlock* CreateBlock(const SourceLocation &Loc);
        ASTIfBlock *CreateIfBlock(const SourceLocation &Loc, ASTExpr *Condition);
        ASTElsifBlock *CreateElsifBlock(const SourceLocation &Loc, ASTExpr *Condition);
        ASTElseBlock *CreateElseBlock(const SourceLocation &Loc);
        ASTSwitchBlock *CreateSwitchBlock(const SourceLocation &Loc, ASTExpr *Expr);
        ASTSwitchCaseBlock *CreateSwitchCaseBlock(const SourceLocation &Loc, ASTExpr *Condition);
        ASTSwitchDefaultBlock *CreateSwitchDefaultBlock(const SourceLocation &Loc);
        ASTWhileBlock *CreateWhileBlock(const SourceLocation &Loc, ASTExpr *Condition);
        ASTForBlock *CreateForBlock(const SourceLocation &Loc, ASTBlock *PostBlock, ASTBlock *LoopBlock);

        // Add Node & NameSpace
        ASTNameSpace *AddNameSpace(const std::string &Name, bool ExternLib = false);
        bool AddNode(ASTNode *Node);

        // Add Top definitions
        bool AddImport(ASTNode *Node, ASTImport *Import);
        bool AddGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar, ASTExpr *Expr);
        bool AddFunction(ASTNode *Node, ASTFunction *Function);
        bool InsertFunction(ASTNodeBase *Base, ASTFunction *Function);
        bool AddFunctionParam(ASTFunction *Function, ASTParam *Param, ASTExpr *Expr);
        void AddFunctionVarParams(ASTFunction *Function, ASTParam *Param); // TODO
        bool AddClass(ASTNode *Node, ASTClass *Class);
        bool AddComment(ASTTopDef *Top, std::string &Comment);
        bool AddExternalGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar);
        bool AddExternalFunction(ASTNode *Node, ASTFunction *Call);

        // Add Value to Array
        bool AddArrayValue(ASTArrayValue *Array, ASTValue *Value);

        // Add Stmt
        bool AddStmt(ASTBlock *Block, ASTStmt *Stmt);
        bool AddStmt(ASTBlock *Block, ASTExprStmt *ExprStmt, ASTExpr *Expr);
        bool AddLocalVar(ASTBlock *Block, ASTLocalVar *LocalVar, ASTExpr *Expr);
        bool AddFunctionCallArg(ASTFunctionCall *Call, ASTArg *Arg, ASTExpr *Expr);

        // Add Blocks structures
        bool AddBlock(ASTBlock *Parent, ASTBlock *Block, bool AddToContent = true);
        bool AddIfBlock(ASTBlock *Block, ASTIfBlock *IfBlock);
        bool AddElsifBlock(ASTIfBlock *IfBlock, ASTElsifBlock *ElsifBlock);
        bool AddElseBlock(ASTIfBlock *IfBlock, ASTElseBlock *ElseBlock);
        bool AddSwitchBlock(ASTBlock *Parent, ASTSwitchBlock *Block);
        bool AddSwitchCaseBlock(ASTSwitchBlock *SwitchBlock, ASTSwitchCaseBlock *CaseBlock);
        bool setSwitchDefaultBlock(ASTSwitchBlock *SwitchBlock, ASTSwitchDefaultBlock *DefaultBlock);
        bool AddWhileBlock(ASTBlock *Block, ASTWhileBlock *WhileBlock);
        bool AddForBlock(ASTBlock *Parent, ASTForBlock *ForBlock, ASTExpr *Condition, ASTBlock *PostBlock, ASTBlock *LoopBlock);

        bool OnCloseBlock(ASTBlock *Block);
    };

}  // end namespace fly

#endif