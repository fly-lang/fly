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

#include "ASTDeclStmt.h"
#include "ASTLoopInStmt.h"
#include "ASTVar.h"
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

    class ASTNullValue;

    class ASTUnsetValue;

    class ASTBoolValue;

    class ASTArrayValue;

    class ASTStructValue;

    class ASTNumberValue;

    class ASTBreakStmt;

    class ASTReturnStmt;

    class ASTContinueStmt;

    class ASTExpr;

    class ASTUnary;

    class ASTBinary;

    class ASTTernary;

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

    enum class ASTUnaryKind;

    enum class ASTBinaryKind;

    enum class ASTCallKind;

    class ASTBuilder {

        DiagnosticsEngine &Diags;

    public:

        explicit ASTBuilder(DiagnosticsEngine &Diags);

        // Create Module
        static ASTModule *CreateModule(InputFile *F);

        static ASTModule *CreateHeader(InputFile *F);

        static ASTComment *CreateComment(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Content);

        // Create Name
	static ASTName *CreateName(llvm::StringRef Name, const SourceLocation &Loc);

        // Create NameSpace
        static ASTNameSpace *CreateNameSpace(ASTModule *Module, const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names);

        // Create Import
        static ASTImport *CreateImport(ASTModule *Module, const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names, llvm::SmallVector<ASTName *, 4> Alias, bool Wildcard = false);

        static ASTFunction *CreateFunction(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name,
                                    llvm::SmallVector<ASTModifier *, 8> &Modifiers, llvm::SmallVector<ASTParam *, 8> &Params,
                                    ASTBlockStmt *Body = nullptr);

        static ASTClass *CreateClass(ASTModule *Module, const SourceLocation &Loc, ASTClassKind ClassKind, llvm::StringRef Name,
                              llvm::SmallVector<ASTModifier *, 8> &Modifiers, llvm::SmallVector<ASTType *, 4> &Bases);

        static ASTAttribute *CreateClassAttribute(const SourceLocation &Loc, ASTClass *Class, ASTType *TypeRef,
                                                llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                                                ASTExpr *Expr = nullptr);

        static ASTMethod *CreateDefaultConstructor(ASTClass *Class);

        static ASTMethod *CreateClassMethod(const SourceLocation &Loc, ASTClass *Class,
                                          llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                                          llvm::SmallVector<ASTParam *, 8> &Params, ASTBlockStmt *Body = nullptr);

        static ASTEnum *CreateEnum(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                   llvm::SmallVector<ASTType *, 4> EnumTypes);

        static ASTEnumEntry *CreateEnumEntry(const SourceLocation &Loc, ASTEnum *Enum, llvm::StringRef Name,
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

        static ASTType *CreateErrorType(const SourceLocation &Loc);

        static ASTArrayType *CreateArrayType(const SourceLocation &Loc, ASTType *ElementType, ASTExpr *SizeExpr);

        static ASTType *CreateType(const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names);

        // Create Values

        static ASTNullValue *CreateNullValue(const SourceLocation &Loc);

        static ASTUnsetValue *CreateUnsetValue(const SourceLocation &Loc);

        static ASTBoolValue *CreateBoolValue(const SourceLocation &Loc, bool Val);

        static ASTNumberValue *CreateNumberValue(const SourceLocation &Loc, llvm::StringRef Val);

        static ASTStringValue *CreateStringValue(const SourceLocation &Loc, llvm::StringRef Val);

        static ASTArrayValue *CreateArrayValue(const SourceLocation &Loc, llvm::SmallVector<ASTValue *, 8> Values);

        static ASTStructValue *CreateStructValue(const SourceLocation &Loc, llvm::StringMap<ASTValue *>);

        // Create Var

         static ASTParam *CreateParam(const SourceLocation &Loc, ASTType *TypeRef, llvm::StringRef Name,
                              llvm::SmallVector<ASTModifier *, 8> &Modifiers, ASTValue *DefaultValue = nullptr);

         static ASTLocalVar *CreateLocalVar(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                    llvm::SmallVector<ASTModifier *, 8> &Modifiers);

        // Create Call

         static ASTCall *CreateCall(const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args, ASTCallKind CallKind, ASTExpr *Parent = nullptr);

         static ASTIdentifier *CreateIdentifier(ASTVar *Var);

         static ASTIdentifier *CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTExpr *Parent = nullptr);

         static ASTMember *CreateMember(const SourceLocation &Loc, llvm::StringRef Name, ASTExpr *Parent);

         static ASTUnary *CreateUnary(const SourceLocation &Loc, ASTUnaryKind OpKind, ASTExpr *Expr);

         static ASTBinary *CreateBinary(const SourceLocation &OpLocation, ASTBinaryKind OpKind,
                                            ASTExpr *LeftExpr, ASTExpr *RightExpr);

         static ASTTernary *CreateTernary(ASTExpr *ConditionExpr,
                                              const SourceLocation &TrueOpLocation, ASTExpr *TrueExpr,
                                              const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr);

        // Create Statements

        static ASTDeclStmt *CreateDeclStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTLocalVar *Var);

        static ASTReturnStmt *CreateReturnStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

        static ASTExprStmt *CreateExprStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

        static ASTFailStmt *CreateFailStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

         static ASTHandleStmt *CreateHandleStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTBlockStmt *BlockStmt);

         static ASTBreakStmt *CreateBreakStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

         static ASTContinueStmt *CreateContinueStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

         static ASTDeleteStmt *CreateDeleteStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTIdentifier *VarRef);

         static ASTBlockStmt *CreateBody(ASTFunction *Function, ASTBlockStmt *Body);

         static ASTBlockStmt *CreateBlockStmt(const SourceLocation &Loc);

         static ASTBlockStmt *CreateBlockStmt(ASTStmt *Parent, const SourceLocation &Loc);

    };

}  // end namespace fly

#endif // FLY_AST_BUILDER_H
