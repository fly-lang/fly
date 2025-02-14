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

#include <AST/ASTVarStmt.h>

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"

namespace fly {

    class Sema;

    class SemaBuilderScopes;

    class SemaBuilderStmt;

    class SemaBuilderIfStmt;

    class SemaBuilderSwitchStmt;

    class SemaBuilderLoopStmt;

    class DiagnosticsEngine;

    class DiagnosticBuilder;

    class SourceLocation;

    class CodeGen;

    class SymTable;

    class SymNameSpace;

    class SymType;

    class ASTModule;

    class ASTImport;

    class ASTComment;

    class ASTClass;

    class ASTFunction;

    class ASTFunction;

    class ASTRef;

    class ASTHandleStmt;

    class ASTCall;

    class ASTArg;

    class ASTStmt;

    class ASTBlockStmt;

    class ASTDeleteStmt;

    class ASTIfStmt;

    class ASTRuleStmt;

    class ASTSwitchStmt;

    class ASTLoopStmt;

    class ASTExprStmt;

    class ASTVarStmt;

    class ASTVar;

    class ASTVarRef;

    class ASTArg;

    class ASTTypeRef;

    class ASTAlias;

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

    class ASTTypeRef;

    class ASTVoidType;

    class ASTValue;

    class ASTBoolValue;

    class ASTIntegerValue;

    class ASTFloatingValue;

    class ASTNullValue;

    class ASTZeroValue;

    class ASTBoolValue;

    class ASTArrayValue;

    class ASTStructValue;

    class ASTIntegerValue;

    class ASTFloatingValue;

    class ASTBreakStmt;

    class ASTReturnStmt;

    class ASTContinueStmt;

    class ASTExpr;

    class ASTValueExpr;

    class ASTVarRefExpr;

    class ASTCallExpr;

    class ASTUnaryOperatorExpr;

    class ASTUnaryOpExpr;

    class ASTBinaryOperatorExpr;

    class ASTBinaryOpExpr;

    class ASTTernaryOperatorExpr;

    class ASTTernaryOpExpr;

    class ASTEnum;

    class ASTScope;

    class ASTTypeRef;

    class ASTFailStmt;

    class ASTStringValue;

    class ASTCharValue;

    class ASTNameSpace;

    class ASTStringType;

    class ASTCharType;

    class ASTErrorType;

    class ASTArrayTypeRef;

    class ASTNameSpaceRef;

    class SymVar;
    
    enum class ASTClassKind;

    enum class ASTUnaryOpExprKind;

    enum class ASTBinaryOpExprKind;

    enum class ASTTernaryOpExprKind;

    enum class ASTCallKind;

    class ASTBuilder {

        friend class Sema;

        friend class SemaResolver;

        static const uint8_t DEFAULT_INTEGER_RADIX;

        static const llvm::StringRef DEFAULT_INTEGER_VALUE;

        static const llvm::StringRef DEFAULT_FLOATING_VALUE;

        Sema &S;

        uint64_t ModuleIdCounter = 0;

    public:

        explicit ASTBuilder(Sema &S);

        // Create Module
        ASTModule *CreateModule(const std::string &Name);

        ASTModule *CreateHeaderModule(const std::string &Name);

        ASTComment *CreateComment(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Content);

        // Create NameSpace
        ASTNameSpace *CreateNameSpace(const SourceLocation &Loc, llvm::StringRef Name, ASTModule *Module = nullptr);

        // Create NameSpace
        ASTNameSpace *CreateNameSpace(const SourceLocation &Loc, llvm::SmallVector<llvm::StringRef, 4> &Names, ASTModule *Module);

        // Create Import
        ASTImport *CreateImport(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name, ASTAlias *Alias);

        // Create Import
        ASTImport *CreateImport(ASTModule *Module, const SourceLocation &Loc, llvm::SmallVector<llvm::StringRef, 4> &Names, ASTAlias *Alias);

        // Create Alias
        ASTAlias * CreateAlias(const SourceLocation &Loc, llvm::StringRef Name);

        ASTVar *CreateGlobalVar(ASTModule *Module, const SourceLocation &Loc, ASTTypeRef *Type, llvm::StringRef Name,
                                      llvm::SmallVector<ASTScope *, 8> &Scopes, ASTExpr *Expr = nullptr);

        ASTFunction *CreateFunction(ASTModule *Module, const SourceLocation &Loc, ASTTypeRef *Type, llvm::StringRef Name,
                                    llvm::SmallVector<ASTScope *, 8> &Scopes, llvm::SmallVector<ASTVar *, 8> &Params,
                                    ASTBlockStmt *Body = nullptr);

        ASTClass *CreateClass(ASTModule *Module, const SourceLocation &Loc, ASTClassKind ClassKind, llvm::StringRef Name,
                              llvm::SmallVector<ASTScope *, 8> &Scopes, llvm::SmallVector<ASTTypeRef *, 4> &SuperClasses);

        ASTVar *CreateClassAttribute(const SourceLocation &Loc, ASTClass *Class, ASTTypeRef *TypeRef,
                                                llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                                                ASTExpr *Expr = nullptr);

        ASTFunction *CreateClassMethod(const SourceLocation &Loc, ASTClass *Class, ASTTypeRef *TypeRef,
                                          llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                                          llvm::SmallVector<ASTVar *, 8> &Params, ASTBlockStmt *Body = nullptr);

        ASTEnum *CreateEnum(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                   llvm::SmallVector<ASTTypeRef *, 4> EnumTypes);

        ASTVar *CreateEnumEntry(const SourceLocation &Loc, ASTEnum *Enum, llvm::StringRef Name,
                                      llvm::SmallVector<ASTScope *, 8> &Scopes);

        // Create Types

        ASTTypeRef *CreateBoolTypeRef(const SourceLocation &Loc);

        ASTTypeRef *CreateByteTypeRef(const SourceLocation &Loc);

        ASTTypeRef *CreateUShortTypeRef(const SourceLocation &Loc);

        ASTTypeRef *CreateShortTypeRef(const SourceLocation &Loc);

        ASTTypeRef *CreateUIntTypeRef(const SourceLocation &Loc);

        ASTTypeRef *CreateIntTypeRef(const SourceLocation &Loc);

        ASTTypeRef *CreateULongTypeRef(const SourceLocation &Loc);

        ASTTypeRef *CreateLongTypeRef(const SourceLocation &Loc);

        ASTTypeRef *CreateFloatTypeRef(const SourceLocation &Loc);

        ASTTypeRef *CreateDoubleTypeRef(const SourceLocation &Loc);

        ASTTypeRef *CreateVoidTypeRef(const SourceLocation &Loc);

        ASTTypeRef *CreateStringTypeRef(const SourceLocation &Loc);

        ASTTypeRef *CreateCharTypeRef(const SourceLocation &Loc);

        ASTTypeRef *CreateErrorTypeRef(const SourceLocation &Loc);

        ASTArrayTypeRef *CreateArrayTypeRef(const SourceLocation &Loc, ASTTypeRef *TypeRef, ASTExpr *Size);

        ASTTypeRef *CreateTypeRef(const SourceLocation &Loc, llvm::StringRef Name, ASTNameSpaceRef *NameSpaceRef = nullptr);

        ASTNameSpaceRef *CreateNameSpaceRef(const SourceLocation &Loc, llvm::SmallVector<llvm::StringRef, 4> &Names);

        // Create Values

        ASTValue *CreateDefaultValue(SymType *Type);

        ASTNullValue *CreateNullValue(const SourceLocation &Loc);

        ASTZeroValue *CreateZeroValue(const SourceLocation &Loc);

        ASTBoolValue *CreateBoolValue(const SourceLocation &Loc, bool Val);

        ASTIntegerValue *CreateIntegerValue(const SourceLocation &Loc, llvm::StringRef Val, uint8_t Radix);

        ASTIntegerValue *CreateIntegerValue(const SourceLocation &Loc, llvm::StringRef Val);

        ASTFloatingValue *CreateFloatingValue(const SourceLocation &Loc, llvm::StringRef Val);

        ASTArrayValue *CreateArrayValue(const SourceLocation &Loc, llvm::SmallVector<ASTValue *, 8> Values);

        ASTCharValue *CreateCharValue(const SourceLocation &Loc, llvm::StringRef Val);

        ASTStringValue *CreateStringValue(const SourceLocation &Loc, llvm::StringRef Val);

        ASTStructValue *CreateStructValue(const SourceLocation &Loc, llvm::StringMap<ASTValue *>);

        ASTVar *CreateParam(const SourceLocation &Loc, ASTTypeRef *TypeRef, llvm::StringRef Name,
                              llvm::SmallVector<ASTScope *, 8> &Scopes, ASTValue *DefaultValue = nullptr);

        ASTVar *CreateErrorHandlerParam();

        ASTVar *CreateLocalVar(ASTBlockStmt *BlockStmt, const SourceLocation &Loc, ASTTypeRef *Type, llvm::StringRef Name,
                                    llvm::SmallVector<ASTScope *, 8> &Scopes);

        // Create Call

        ASTCall *CreateCall(const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args, ASTCallKind CallKind, ASTRef *Parent = nullptr);

        ASTCall *CreateCall(ASTFunction *Function, llvm::SmallVector<ASTExpr *, 8> &Args);

        ASTCall *CreateCall(ASTRef *Instance, ASTFunction *Method);

        // Create VarRef
        ASTVarRef *CreateVarRef(ASTRef *Ref);

        ASTVarRef *CreateVarRef(ASTVar *Var);

        ASTRef *CreateUndefinedRef(const SourceLocation &Loc, llvm::StringRef Name, ASTRef *Parent = nullptr);

        // Create Expressions

        ASTValueExpr *CreateExpr(ASTValue *Value);

        ASTCallExpr *CreateExpr(ASTCall *Call);

        ASTVarRefExpr *CreateExpr(ASTVarRef *VarRef);

        ASTUnaryOpExpr *CreateUnaryOpExpr(const SourceLocation &Loc, ASTUnaryOpExprKind OpKind, ASTExpr *Expr);

        ASTBinaryOpExpr *CreateBinaryOpExpr(const SourceLocation &OpLocation, ASTBinaryOpExprKind OpKind,
                                            ASTExpr *LeftExpr, ASTExpr *RightExpr);

        ASTTernaryOpExpr *CreateTernaryOpExpr(ASTExpr *ConditionExpr,
                                              const SourceLocation &TrueOpLocation, ASTExpr *TrueExpr,
                                              const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr);

        // Create Statements

        SemaBuilderStmt *CreateAssignmentStmt(ASTBlockStmt *Parent, ASTVarRef *VarRef,
                                              ASTAssignOperatorKind Kind = ASTAssignOperatorKind::EQUAL);

        SemaBuilderStmt *CreateAssignmentStmt(ASTBlockStmt *Parent, ASTVar *Var,
                                              ASTAssignOperatorKind Kind = ASTAssignOperatorKind::EQUAL);

        SemaBuilderStmt *CreateReturnStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

        SemaBuilderStmt *CreateExprStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

        SemaBuilderStmt *CreateFailStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

        ASTHandleStmt *CreateHandleStmt(ASTBlockStmt *Parent, const SourceLocation &Loc,
            ASTBlockStmt *BlockStmt, ASTVarRef *ErrorRef = nullptr);

        ASTBreakStmt *CreateBreakStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

        ASTContinueStmt *CreateContinueStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

        ASTDeleteStmt *CreateDeleteStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTVarRef *VarRef);

        // Create Blocks structures

        ASTBlockStmt *CreateBody(ASTFunction *FunctionBase, ASTBlockStmt *Body);

        ASTBlockStmt *CreateBlockStmt(const SourceLocation &Loc);

        ASTBlockStmt *CreateBlockStmt(ASTStmt *Parent, const SourceLocation &Loc);

        SemaBuilderIfStmt *CreateIfBuilder(ASTBlockStmt *Parent);

        SemaBuilderSwitchStmt *CreateSwitchBuilder(ASTBlockStmt *Parent);

        SemaBuilderLoopStmt *CreateLoopBuilder(ASTBlockStmt *Parent, const SourceLocation &Loc);
    };

}  // end namespace fly

#endif