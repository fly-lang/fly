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
#include <AST/ASTAssignStmt.h>
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

    class ASTAttribute;

    class ASTMethod;

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

    class ASTAssignStmt;

    class ASTParam;

    class ASTLocalVar;

    class ASTArg;

    class ASTParam;

    class ASTType;

    class ASTMember;

    class ASTType;

    class ASTValue;

    class ASTBoolValue;

    class ASTNumberValue;

    class ASTDefaultValue;

    class ASTNullValue;

    class ASTBoolValue;

    class ASTArrayValue;

    class ASTStructValue;

    class ASTNumberValue;

    class ASTBreakStmt;

    class ASTReturnStmt;

    class ASTContinueStmt;

    class ASTExpr;

    class ASTUnaryOp;

    class ASTBinaryOp;

    class ASTTernaryOp;

    class ASTEnum;

    class ASTEnumEntry;

    class ASTModifier;

    class ASTType;

    class ASTFailStmt;

    class ASTStringValue;

    class ASTNameSpace;

    class ASTName;

    class ASTArrayType;

    class InputFile;

    enum class ASTClassKind;

    enum class ASTUnaryOpKind;

    enum class ASTBinaryOpKind;

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
                                    llvm::SmallVector<ASTModifier *, 8> &Modifiers, llvm::SmallVector<ASTParam *, 8> &Params,
                                    ASTBlockStmt *Body = nullptr);

        ASTClass *CreateClass(ASTModule *Module, const SourceLocation &Loc, ASTClassKind ClassKind, llvm::StringRef Name,
                              llvm::SmallVector<ASTModifier *, 8> &Modifiers, llvm::SmallVector<ASTType *, 4> &Bases);

        ASTAttribute *CreateClassAttribute(const SourceLocation &Loc, ASTClass *Class, ASTType *TypeRef,
                                                llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                                                ASTExpr *Expr = nullptr);

        static ASTMethod *CreateDefaultConstructor(ASTClass *Class);

        ASTMethod *CreateClassMethod(const SourceLocation &Loc, ASTClass *Class, ASTType *ReturnType,
                                          llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                                          llvm::SmallVector<ASTParam *, 8> &Params, ASTBlockStmt *Body = nullptr);

        ASTEnum *CreateEnum(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                   llvm::SmallVector<ASTType *, 4> EnumTypes);

        ASTEnumEntry *CreateEnumEntry(const SourceLocation &Loc, ASTEnum *Enum, llvm::StringRef Name,
                                      llvm::SmallVector<ASTModifier *, 8> &Modifiers);

        // Create Modifiers
        static ASTModifier *CreateModifier(const SourceLocation &Loc, ASTModifierKind Kind);

        // Create Types

        static ASTType *CreateBoolType(const SourceLocation &Loc);

        static ASTType *CreateByteType(const SourceLocation &Loc);

        static ASTType *CreateUShortType(const SourceLocation &Loc);

        static ASTType *CreateShortType(const SourceLocation &Loc);

        static ASTType *CreateUIntType(const SourceLocation &Loc);

        static ASTType *CreateIntType(const SourceLocation &Loc);

        static ASTType *CreateULongType(const SourceLocation &Loc);

        static ASTType *CreateLongType(const SourceLocation &Loc);

        static ASTType *CreateFloatType(const SourceLocation &Loc);

        static ASTType *CreateDoubleType(const SourceLocation &Loc);

        static ASTType *CreateVoidType(const SourceLocation &Loc);

        static ASTType *CreateStringType(const SourceLocation &Loc);

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

         ASTParam *CreateParam(const SourceLocation &Loc, ASTType *TypeRef, llvm::StringRef Name,
                              llvm::SmallVector<ASTModifier *, 8> &Modifiers, ASTValue *DefaultValue = nullptr);

         ASTLocalVar *CreateLocalVar(ASTBlockStmt *BlockStmt, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                    llvm::SmallVector<ASTModifier *, 8> &Modifiers);

        // Create Call

         ASTCall *CreateCall(const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args, ASTCallKind CallKind, ASTExpr *Parent = nullptr);

         ASTIdentifier *CreateIdentifier(ASTVar *Var);

         ASTIdentifier *CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTExpr *Parent = nullptr);

        ASTMember *CreateMember(ASTVar *Var, ASTExpr *Parent);

         ASTMember *CreateMember(const SourceLocation &Loc, llvm::StringRef Name, ASTExpr *Parent);

         ASTUnaryOp *CreateUnary(const SourceLocation &Loc, ASTUnaryOpKind OpKind, ASTExpr *Expr);

         ASTBinaryOp *CreateBinary(const SourceLocation &OpLocation, ASTBinaryOpKind OpKind,
                                            ASTExpr *LeftExpr, ASTExpr *RightExpr);

         ASTTernaryOp *CreateTernary(ASTExpr *ConditionExpr,
                                              const SourceLocation &TrueOpLocation, ASTExpr *TrueExpr,
                                              const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr);

        // Create Statements

        ASTAssignStmt *CreateAssignmentStmt(ASTBlockStmt *Parent, ASTExpr *Source,
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

         static ASTBlockStmt *CreateBody(ASTFunction *Function, ASTBlockStmt *Body);

         static ASTBlockStmt *CreateBlockStmt(const SourceLocation &Loc);

         static ASTBlockStmt *CreateBlockStmt(ASTStmt *Parent, const SourceLocation &Loc);

    };

}  // end namespace fly

#endif // FLY_AST_BUILDER_H

