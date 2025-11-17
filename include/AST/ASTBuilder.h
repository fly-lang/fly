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

    class ASTIdentifier;

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

    class ASTDefaultValue;

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

    class ASTName;

    class ASTArrayType;

    class InputFile;

    enum class ASTClassKind;

    enum class ASTUnaryOpExprKind;

    enum class ASTBinaryOpExprKind;

    enum class ASTTernaryOpExprKind;

    enum class ASTCallKind;

    class ASTBuilder {

        DiagnosticsEngine &Diags;

    public:

        explicit ASTBuilder(DiagnosticsEngine &Diags);

        // Create Module
        ASTModule *CreateModule(InputFile *F);

        ASTModule *CreateHeader(InputFile *F);

        ASTComment *CreateComment(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Content);

        // Create Name
		ASTName *CreateName(llvm::StringRef Name, const SourceLocation &Loc);

        // Create NameSpace
        ASTNameSpace *CreateNameSpace(ASTModule *Module, const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names);

        // Create Import
        ASTImport *CreateImport(ASTModule *Module, const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names, llvm::SmallVector<ASTName *, 4> Alias);

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
        static ASTModifier *CreateModifier(const SourceLocation &Loc, ASTModifierKind Kind);


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

        ASTType *CreateStringType(const SourceLocation &Loc);

        ASTType *CreateErrorType(const SourceLocation &Loc);

        ASTArrayType *CreateArrayType(const SourceLocation &Loc, ASTType *ElementType, ASTExpr *SizeExpr);

        ASTType *CreateType(const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names);

        // Create Values

        static ASTDefaultValue *CreateDefaultValue();

        static ASTNullValue *CreateNullValue(const SourceLocation &Loc);

        static ASTBoolValue *CreateBoolValue(const SourceLocation &Loc, bool Val);

        static ASTNumberValue *CreateNumberValue(const SourceLocation &Loc, llvm::StringRef Val);

        static ASTStringValue *CreateStringValue(const SourceLocation &Loc, llvm::StringRef Val);

        static ASTArrayValue *CreateArrayValue(const SourceLocation &Loc, llvm::SmallVector<ASTValue *, 8> Values);

        static ASTStructValue *CreateStructValue(const SourceLocation &Loc, llvm::StringMap<ASTValue *>);

        // Create Var

         ASTVar *CreateParam(const SourceLocation &Loc, ASTType *TypeRef, llvm::StringRef Name,
                              llvm::SmallVector<ASTModifier *, 8> &Modifiers, ASTValue *DefaultValue = nullptr);

         ASTVar *CreateLocalVar(ASTBlockStmt *BlockStmt, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                    llvm::SmallVector<ASTModifier *, 8> &Modifiers);

        // Create Call

         ASTCall *CreateCall(const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args, ASTCallKind CallKind, ASTExpr *Parent = nullptr);

         ASTCall *CreateCall(llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args);

         ASTCall *CreateCall(ASTIdentifier *Instance, llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args);

         ASTIdentifier *CreateIdentifier(ASTVar *Var, ASTIdentifier *Parent = nullptr);

         ASTIdentifier *CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTIdentifier *Parent = nullptr);

         ASTIdentifier *CreateMember(const SourceLocation &Loc, llvm::StringRef Name, ASTExpr *Parent);

         ASTUnaryOpExpr *CreateUnary(const SourceLocation &Loc, ASTUnaryOpExprKind OpKind, ASTExpr *Expr);

         ASTBinaryOpExpr *CreateBinary(const SourceLocation &OpLocation, ASTBinaryOpExprKind OpKind,
                                            ASTExpr *LeftExpr, ASTExpr *RightExpr);

         ASTTernaryOpExpr *CreateTernary(ASTExpr *ConditionExpr,
                                              const SourceLocation &TrueOpLocation, ASTExpr *TrueExpr,
                                              const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr);

        // Create Statements

        ASTVarStmt *CreateAssignmentStmt(ASTBlockStmt *Parent, ASTIdentifier *VarRef,
                                              ASTAssignOperatorKind Kind = ASTAssignOperatorKind::EQUAL);

        ASTVarStmt *CreateAssignmentStmt(ASTBlockStmt *Parent, ASTVar *Var,
                                              ASTAssignOperatorKind Kind = ASTAssignOperatorKind::EQUAL);

        ASTReturnStmt *CreateReturnStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

        ASTExprStmt *CreateExprStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

        ASTFailStmt *CreateFailStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

         ASTHandleStmt *CreateHandleStmt(ASTBlockStmt *Parent, const SourceLocation &Loc,
            ASTBlockStmt *BlockStmt, ASTIdentifier *ErrorRef = nullptr);

         ASTBreakStmt *CreateBreakStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

         ASTContinueStmt *CreateContinueStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

         ASTDeleteStmt *CreateDeleteStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTIdentifier *VarRef);

        // Create Blocks structures

         ASTBlockStmt *CreateBody(ASTFunction *FunctionBase, ASTBlockStmt *Body);

         ASTBlockStmt *CreateBlockStmt(const SourceLocation &Loc);

         ASTBlockStmt *CreateBlockStmt(ASTStmt *Parent, const SourceLocation &Loc);

    };

}  // end namespace fly

#endif // FLY_AST_BUILDER_H