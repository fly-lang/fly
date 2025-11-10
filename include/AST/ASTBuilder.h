//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBuilder.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BUILDER_H
#define FLY_AST_BUILDER_H

#include <AST/ASTModifier.h>
#include <AST/ASTVarStmt.h>
#include "llvm/ADT/StringMap.h"

namespace fly {

    class DiagnosticsEngine;

    class DiagnosticBuilder;

    class SourceLocation;

    class CodeGen;

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

    class ASTArg;

    class ASTType;

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

    class ASTType;

    class ASTVoidType;

    class ASTValue;

    class ASTBoolValue;

    class ASTNumberValue;

    class ASTNullValue;

    class ASTSizeValue;

    class ASTBoolValue;

    class ASTArrayValue;

    class ASTStructValue;

    class ASTNumberValue;

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

    class ASTModifier;

    class ASTType;

    class ASTFailStmt;

    class ASTStringValue;

    class ASTNameSpace;

    class ASTArrayType;

    class ASTNameSpaceRef;

    class InputFile;

    enum class ASTClassKind;

    enum class ASTUnaryOpExprKind;

    enum class ASTBinaryOpExprKind;

    enum class ASTTernaryOpExprKind;

    enum class ASTCallKind;

    class ASTBuilder {

        static ASTBoolValue DEFAULT_BOOL_VALUE;

        static ASTNumberValue DEFAULT_INTEGER_VALUE;

        static ASTNumberValue DEFAULT_FLOATING_VALUE;

        static ASTStringValue DEFAULT_STRING_VALUE;

        static ASTArrayValue DEFAULT_ARRAY_VALUE;

        static ASTNullValue DEFAULT_NULL_VALUE;

        DiagnosticsEngine &Diags;

    public:

        explicit ASTBuilder(DiagnosticsEngine &Diags);

        // Create Module
        ASTModule *CreateModule(InputFile *F);

        ASTModule *CreateHeader(InputFile *F);

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

          ASTFunction *CreateFunction(ASTModule *Module, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                    llvm::SmallVector<ASTModifier *, 8> &Modifiers, llvm::SmallVector<ASTVar *, 8> &Params,
                                    ASTBlockStmt *Body = nullptr);

          ASTClass *CreateClass(ASTModule *Module, const SourceLocation &Loc, ASTClassKind ClassKind, llvm::StringRef Name,
                              llvm::SmallVector<ASTModifier *, 8> &Modifiers, llvm::SmallVector<ASTType *, 4> &SuperClasses);

          ASTVar *CreateClassAttribute(const SourceLocation &Loc, ASTClass *Class, ASTType *TypeRef,
                                                llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                                                ASTExpr *Expr = nullptr);

          ASTFunction *CreateClassMethod(const SourceLocation &Loc, ASTClass *Class, ASTType *ReturnTypeRef,
                                          llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                                          llvm::SmallVector<ASTVar *, 8> &Params, ASTBlockStmt *Body = nullptr);

          ASTEnum *CreateEnum(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                   llvm::SmallVector<ASTType *, 4> EnumTypes);

          ASTVar *CreateEnumEntry(const SourceLocation &Loc, ASTEnum *Enum, llvm::StringRef Name,
                                      llvm::SmallVector<ASTModifier *, 8> &Modifiers);

        // Create Modifiers
          ASTModifier *CreateModifier(const SourceLocation &Loc, ASTModifierKind Kind);


        // Create Types

          ASTType *CreateBoolTypeRef(const SourceLocation &Loc);

          ASTType *CreateByteTypeRef(const SourceLocation &Loc);

          ASTType *CreateUShortTypeRef(const SourceLocation &Loc);

          ASTType *CreateShortTypeRef(const SourceLocation &Loc);

          ASTType *CreateUIntTypeRef(const SourceLocation &Loc);

          ASTType *CreateIntTypeRef(const SourceLocation &Loc);

          ASTType *CreateULongTypeRef(const SourceLocation &Loc);

          ASTType *CreateLongTypeRef(const SourceLocation &Loc);

          ASTType *CreateFloatTypeRef(const SourceLocation &Loc);

          ASTType *CreateDoubleTypeRef(const SourceLocation &Loc);

          ASTType *CreateVoidTypeRef(const SourceLocation &Loc);

          ASTType *CreateStringTypeRef(const SourceLocation &Loc);

          ASTType *CreateErrorTypeRef(const SourceLocation &Loc);

          ASTArrayType *CreateArrayTypeRef(const SourceLocation &Loc, ASTType *TypeRef);

          ASTNameSpaceRef *CreateNameSpaceRef(const SourceLocation &Loc, llvm::SmallVector<llvm::StringRef, 4> Names);

        // Create Values

        static ASTNullValue *CreateNullValue(const SourceLocation &Loc);

        static ASTBoolValue *CreateBoolValue(const SourceLocation &Loc, bool Val);

        static ASTNumberValue *CreateNumberValue(const SourceLocation &Loc, llvm::StringRef Val);

        static ASTArrayValue *CreateArrayValue(const SourceLocation &Loc, llvm::SmallVector<ASTValue *, 8> Values);

        static ASTStringValue *CreateStringValue(const SourceLocation &Loc, llvm::StringRef Val);

        static ASTStructValue *CreateStructValue(const SourceLocation &Loc, llvm::StringMap<ASTValue *>);

        // Create Var

         ASTVar *CreateParam(const SourceLocation &Loc, ASTType *TypeRef, llvm::StringRef Name,
                              llvm::SmallVector<ASTModifier *, 8> &Modifiers, ASTValue *DefaultValue = nullptr);

         ASTVar *CreateLocalVar(ASTBlockStmt *BlockStmt, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                    llvm::SmallVector<ASTModifier *, 8> &Modifiers);

        // Create Call

         ASTCall *CreateCall(const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args, ASTCallKind CallKind, ASTRef *Parent = nullptr);

         ASTCall *CreateCall(llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args);

         ASTCall *CreateCall(ASTRef *Instance, llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args);

         ASTRef *CreateVarRef(ASTVar *Var, ASTRef *Parent = nullptr);

         ASTRef *CreateVarRef(const SourceLocation &Loc, llvm::StringRef Name, ASTRef *Parent = nullptr);

        // Create Expressions

         ASTValueExpr *CreateExpr(ASTValue *Value);

         ASTCallExpr *CreateExpr(ASTCall *Call);

         ASTVarRefExpr *CreateExpr(ASTRef *VarRef);

         ASTUnaryOpExpr *CreateUnaryOpExpr(const SourceLocation &Loc, ASTUnaryOpExprKind OpKind, ASTExpr *Expr);

         ASTBinaryOpExpr *CreateBinaryOpExpr(const SourceLocation &OpLocation, ASTBinaryOpExprKind OpKind,
                                            ASTExpr *LeftExpr, ASTExpr *RightExpr);

         ASTTernaryOpExpr *CreateTernaryOpExpr(ASTExpr *ConditionExpr,
                                              const SourceLocation &TrueOpLocation, ASTExpr *TrueExpr,
                                              const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr);

         ASTHandleStmt *CreateHandleStmt(ASTBlockStmt *Parent, const SourceLocation &Loc,
            ASTBlockStmt *BlockStmt, ASTRef *ErrorRef = nullptr);

         ASTBreakStmt *CreateBreakStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

         ASTContinueStmt *CreateContinueStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

         ASTDeleteStmt *CreateDeleteStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTRef *VarRef);

        // Create Blocks structures

         ASTBlockStmt *CreateBody(ASTFunction *FunctionBase, ASTBlockStmt *Body);

         ASTBlockStmt *CreateBlockStmt(const SourceLocation &Loc);

         ASTBlockStmt *CreateBlockStmt(ASTStmt *Parent, const SourceLocation &Loc);

    };

}  // end namespace fly

#endif // FLY_AST_BUILDER_H