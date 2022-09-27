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

#include "AST/ASTClass.h"

namespace fly {

    class Sema;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;
    class CodeGen;
    class ASTContext;
    class ASTNameSpace;
    class ASTNode;
    class ASTTopDef;
    class ASTImport;
    class ASTTopScopes;
    class ASTClass;
    class ASTClassScopes;
    class ASTClassField;
    class ASTClassMethod;
    class ASTGlobalVar;
    class ASTFunction;
    class ASTFunctionCall;
    class ASTStmt;
    class ASTBlock;
    class ASTIfBlock;
    class ASTElsifBlock;
    class ASTElseBlock;
    class ASTSwitchBlock;
    class ASTSwitchCaseBlock;
    class ASTSwitchDefaultBlock;
    class ASTWhileBlock;
    class ASTForBlock;
    class ASTForLoopBlock;
    class ASTForPostBlock;
    class ASTExprStmt;
    class ASTVarAssign;
    class ASTParam;
    class ASTLocalVar;
    class ASTVarRef;
    class ASTArg;
    class ASTType;
    class ASTBoolType;
    class ASTByteType;
    class ASTUShortType;
    class ASTShortType;
    class ASTUIntType;
    class ASTIntType;
    class ASTULongType;
    class ASTLongType;
    class ASTFloatType;
    class ASTDoubleType;
    class ASTArrayType;
    class ASTClassType;
    class ASTVoidType;
    class ASTValue;
    class ASTBoolValue;
    class ASTIntegerValue;
    class ASTFloatingValue;
    class ASTNullValue;
    class ASTBoolValue;
    class ASTArrayValue;
    class ASTIntegerValue;
    class ASTFloatingValue;
    class ASTBreak;
    class ASTReturn;
    class ASTContinue;
    class ASTExpr;
    class ASTEmptyExpr;
    class ASTValueExpr;
    class ASTVarRefExpr;
    class ASTFunctionCallExpr;
    class ASTUnaryGroupExpr;
    class ASTBinaryGroupExpr;
    class ASTTernaryGroupExpr;
    enum class UnaryOpKind;
    enum class UnaryOptionKind;
    enum class BinaryOpKind;

    class SemaBuilder {

        friend class SemaResolver;

        Sema &S;

        ASTContext *Context;

    public:

        SemaBuilder(Sema &S);

        bool Build();
        void Destroy();

        // Create Node
        ASTNode *CreateNode(const std::string &Name, std::string &NameSpace);
        ASTNode *CreateHeaderNode(const std::string &Name, std::string &NameSpace);

        // Create Top Definitions
        ASTImport *CreateImport(const SourceLocation &NameLoc, StringRef Name);
        ASTImport *CreateImport(const SourceLocation &NameLoc, StringRef Name,
                                const SourceLocation &AliasLoc, StringRef Alias);
        static ASTTopScopes *CreateTopScopes(ASTVisibilityKind Visibility, bool Constant);
        ASTGlobalVar *CreateGlobalVar(ASTNode *Node, const SourceLocation &Loc, ASTType *Type, const std::string &Name,
                                      ASTTopScopes *Scopes);
        ASTFunction *CreateFunction(ASTNode *Node, const SourceLocation &Loc, ASTType *Type, const std::string &Name,
                                    ASTTopScopes *Scopes);
        ASTClass *CreateClass(ASTNode *Node, const SourceLocation &Loc, const std::string &Name,
                              ASTTopScopes *Scopes);
        static ASTClassScopes *CreateClassScopes(ASTClassVisibilityKind Visibility, bool Constant);

        // Create Types
        static ASTBoolType *CreateBoolType(const SourceLocation &Loc);
        static ASTByteType *CreateByteType(const SourceLocation &Loc);
        static ASTUShortType *CreateUShortType(const SourceLocation &Loc);
        static ASTShortType *CreateShortType(const SourceLocation &Loc);
        static ASTUIntType *CreateUIntType(const SourceLocation &Loc);
        static ASTIntType *CreateIntType(const SourceLocation &Loc);
        static ASTULongType *CreateULongType(const SourceLocation &Loc);
        static ASTLongType *CreateLongType(const SourceLocation &Loc);
        static ASTFloatType *CreateFloatType(const SourceLocation &Loc);
        static ASTDoubleType *CreateDoubleType(const SourceLocation &Loc);
        static ASTVoidType *CreateVoidType(const SourceLocation &Loc);
        static ASTArrayType *CreateArrayType(const SourceLocation &Loc, ASTType *Type, ASTExpr *Size);
        static ASTClassType *CreateClassType(const SourceLocation &Loc, StringRef Name, StringRef NameSpace);
        static ASTClassType *CreateClassType(ASTClass *Class);

        // Create Values
        static ASTNullValue *CreateNullValue(const SourceLocation &Loc);
        static ASTBoolValue *CreateBoolValue(const SourceLocation &Loc, bool Val);
        static ASTIntegerValue *CreateIntegerValue(const SourceLocation &Loc, uint64_t Val, bool Negative = false);
        static ASTIntegerValue *CreateCharValue(const SourceLocation &Loc, char Val);
        static ASTFloatingValue *CreateFloatingValue(const SourceLocation &Loc, std::string Val);
        static ASTFloatingValue *CreateFloatingValue(const SourceLocation &Loc, double Val);
        static ASTArrayValue *CreateArrayValue(const SourceLocation &Loc);
        static ASTValue *CreateDefaultValue(ASTType *Type);

        // Create Statements
        ASTFunctionCall *CreateFunctionCall(ASTStmt *Parent, const SourceLocation &Loc, std::string &Name, std::string &NameSpace);
        ASTFunctionCall *CreateFunctionCall(ASTStmt *Parent, ASTFunction *Function);
        ASTArg *CreateArg(ASTFunctionCall *Call, const SourceLocation &Loc);
        ASTParam *CreateParam(ASTFunction *Function, const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant = false);
        ASTLocalVar *CreateLocalVar(ASTBlock *Parent, const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant = false);
        ASTVarAssign *CreateVarAssign(ASTBlock *Parent, ASTVarRef *VarRef);
        ASTReturn *CreateReturn(ASTBlock *Parent, const SourceLocation &Loc);
        ASTBreak *CreateBreak(ASTBlock *Parent, const SourceLocation &Loc);
        ASTContinue *CreateContinue(ASTBlock *Parent, const SourceLocation &Loc);
        ASTExprStmt *CreateExprStmt(ASTBlock *Parent, const SourceLocation &Loc);

        // Create Var References
        ASTVarRef *CreateVarRef(const SourceLocation &Loc, StringRef Name, StringRef NameSpace);
        ASTVarRef *CreateVarRef(ASTLocalVar *LocalVar);
        ASTVarRef *CreateVarRef(ASTGlobalVar *GlobalVar);

        // Create Expressions
        ASTEmptyExpr *CreateExpr(ASTStmt *Stmt);
        ASTValueExpr *CreateExpr(ASTStmt *Stmt, ASTValue *Value);
        ASTFunctionCallExpr *CreateExpr(ASTStmt *Stmt, ASTFunctionCall *Call);
        ASTVarRefExpr *CreateExpr(ASTStmt *Stmt, ASTVarRef *VarRef);
        ASTUnaryGroupExpr *CreateUnaryExpr(ASTStmt *Stmt, const SourceLocation &Loc, UnaryOpKind Kind,
                                           UnaryOptionKind OptionKind,ASTVarRefExpr *First);
        ASTBinaryGroupExpr *CreateBinaryExpr(ASTStmt *Stmt, const SourceLocation &OpLoc,
                                             BinaryOpKind Kind, ASTExpr *First, ASTExpr *Second);
        ASTTernaryGroupExpr *CreateTernaryExpr(ASTStmt *Stmt, ASTExpr *First, const SourceLocation &IfLoc,
                                               ASTExpr *Second, const SourceLocation &ElseLoc, ASTExpr *Third);

        // Create Blocks structures
        ASTBlock* CreateBlock(ASTBlock *Parent, const SourceLocation &Loc);
        ASTBlock* getBlock(ASTFunction *Function);
        ASTIfBlock *CreateIfBlock(ASTBlock *Parent, const SourceLocation &Loc);
        ASTElsifBlock *CreateElsifBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc);
        ASTElseBlock *CreateElseBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc);
        ASTSwitchBlock *CreateSwitchBlock(ASTBlock *Parent, const SourceLocation &Loc);
        ASTSwitchCaseBlock *CreateSwitchCaseBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc);
        ASTSwitchDefaultBlock *CreateSwitchDefaultBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc);
        ASTWhileBlock *CreateWhileBlock(ASTBlock *Parent, const SourceLocation &Loc);
        ASTForBlock *CreateForBlock(ASTBlock *Parent, const SourceLocation &Loc);
        ASTForLoopBlock *CreateForLoopBlock(ASTForBlock *Parent, const SourceLocation &Loc);
        ASTForPostBlock *CreateForPostBlock(ASTForBlock *Parent, const SourceLocation &Loc);

        // Add Node & NameSpace
        ASTNameSpace *AddNameSpace(const std::string &Name, bool ExternLib = false);
        bool AddNode(ASTNode *Node);

        // Add Top definitions
        bool AddImport(ASTNode *Node, ASTImport *Import);
        bool AddClass(ASTNode *Node, ASTClass *Class);
        bool AddGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar, ASTValue *Value = nullptr);
        bool AddGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar, ASTExpr *Expr);
        bool AddFunction(ASTNode *Node, ASTFunction *Function);
        bool InsertFunction(llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &Functions,
                            ASTFunction *Function);
        bool AddParam(ASTParam *Param);
        void AddFunctionVarParams(ASTFunction *Function, ASTParam *Param); // TODO
        bool AddComment(ASTTopDef *Top, std::string &Comment);
        bool AddExternalGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar);
        bool AddExternalFunction(ASTNode *Node, ASTFunction *Function);

        // Add Value to Array
        bool AddArrayValue(ASTArrayValue *Array, ASTValue *Value);
        bool AddFunctionCallArg(ASTFunctionCall *Call, ASTArg *Arg);

        // Add Stmt
        bool AddStmt(ASTStmt *Stmt);
        bool AddBlock(ASTBlock *Block);

    private:
        bool AddExpr(ASTStmt *Stmt, ASTExpr *Expr);

    };

}  // end namespace fly

#endif