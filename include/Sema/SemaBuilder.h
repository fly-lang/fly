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
#include "Sema/SemaValidator.h"
#include "AST/ASTScopes.h"

#include "llvm/ADT/StringMap.h"

#include <map>

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

    class ASTContext;

    class ASTNameSpace;

    class ASTModule;

    class ASTTopDef;

    class ASTImport;

    class ASTAlias;

    class ASTClass;

    class ASTClassAttribute;

    class ASTGlobalVar;

    class ASTFunction;

    class ASTFunctionBase;

    class ASTIdentifier;

    class ASTHandleStmt;

    class ASTCall;

    class ASTArg;

    class ASTStmt;

    class ASTBlockStmt;

    class ASTDeleteStmt;

    class ASTIfStmt;

    class ASTElsif;

    class ASTSwitchStmt;

    class ASTSwitchCase;

    class ASTLoopStmt;

    class ASTExprStmt;

    class ASTVarStmt;

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

    class ASTEnumType;

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

    class ASTEmptyExpr;

    class ASTValueExpr;

    class ASTVarRefExpr;

    class ASTCallExpr;

    class ASTUnaryOperatorExpr;

    class ASTUnaryGroupExpr;

    class ASTBinaryOperatorExpr;

    class ASTBinaryGroupExpr;

    class ASTTernaryOperatorExpr;

    class ASTTernaryGroupExpr;

    class ASTEnum;

    class ASTEnumEntry;

    class ASTIdentityType;

    class ASTFailStmt;

    class ASTStringValue;

    enum class ASTClassKind;

    enum class ASTUnaryOperatorKind;

    enum class ASTBinaryOperatorKind;

    enum class ASTTernaryOperatorKind;

    enum class ASTCallKind;

    class SemaBuilder {

        friend class Sema;

        friend class SemaResolver;

        Sema &S;

        ASTContext *CreateContext();

        ASTNameSpace *CreateDefaultNameSpace();

    public:

        explicit SemaBuilder(Sema &S);

        // Create Module
        ASTModule *CreateModule(const std::string &Name);

        ASTModule *CreateHeaderModule(const std::string &Name);

        // Create NameSpace
        ASTNameSpace *CreateNameSpace(ASTIdentifier *Identifier, ASTModule *Module = nullptr);

        // Create Top Definitions
        ASTImport *CreateImport(ASTModule *Module, const SourceLocation &Loc, StringRef Name, ASTAlias* Alias);

        ASTAlias *CreateAlias(ASTImport *Import, const SourceLocation &Loc, StringRef Name);

        ASTGlobalVar *CreateGlobalVar(ASTModule *Module, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                      SmallVector<ASTScope *, 8> &Scopes, ASTExpr *Expr = nullptr);

        ASTFunction *CreateFunction(ASTModule *Module, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                    SmallVector<ASTScope *, 8> &Scopes, SmallVector<ASTParam *, 8> &Params,
                                    ASTBlockStmt *Body = nullptr);

        ASTClass *CreateClass(ASTModule *Module, const SourceLocation &Loc, ASTClassKind ClassKind, llvm::StringRef Name,
                              SmallVector<ASTScope *, 8> &Scopes, llvm::SmallVector<ASTClassType *, 4> &ClassTypes);

        ASTClassAttribute *CreateClassAttribute(const SourceLocation &Loc, ASTClass &Class, ASTType *Type,
                                                llvm::StringRef Name, SmallVector<ASTScope *, 8> &Scopes,
                                                ASTExpr *Expr = nullptr);

        ASTClassMethod *CreateClassConstructor(const SourceLocation &Loc, ASTClass &Class,
                                               llvm::SmallVector<ASTScope *, 8> &Scopes,
                                               llvm::SmallVector<ASTParam *, 8> &Params, ASTBlockStmt *Body);

        ASTClassMethod *CreateClassMethod(const SourceLocation &Loc, ASTClass &Class, ASTType *Type,
                                          llvm::StringRef Name, SmallVector<ASTScope *, 8> &Scopes,
                                          llvm::SmallVector<ASTParam *, 8> &Params, ASTBlockStmt *Body);

        ASTClassMethod *CreateClassVirtualMethod(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                                 SmallVector<ASTScope *, 8> &Scopes,
                                                 llvm::SmallVector<ASTParam *, 8> &Params);

        ASTEnum *CreateEnum(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name, SmallVector<ASTScope *, 8> &Scopes,
                   llvm::SmallVector<ASTEnumType *, 4> EnumTypes);

        ASTEnumEntry *CreateEnumEntry(const SourceLocation &Loc, ASTEnum &Enum, llvm::StringRef Name,
                                      llvm::SmallVector<ASTScope *, 8> &Scopes);

        // Create Types
        ASTBoolType *CreateBoolType(const SourceLocation &Loc);

        ASTByteType *CreateByteType(const SourceLocation &Loc);

        ASTUShortType *CreateUShortType(const SourceLocation &Loc);

        ASTShortType *CreateShortType(const SourceLocation &Loc);

        ASTUIntType *CreateUIntType(const SourceLocation &Loc);

        ASTIntType *CreateIntType(const SourceLocation &Loc);

        ASTULongType *CreateULongType(const SourceLocation &Loc);

        ASTLongType *CreateLongType(const SourceLocation &Loc);

        ASTFloatType *CreateFloatType(const SourceLocation &Loc);

        ASTDoubleType *CreateDoubleType(const SourceLocation &Loc);

        ASTVoidType *CreateVoidType(const SourceLocation &Loc);

        ASTArrayType *CreateArrayType(const SourceLocation &Loc, ASTType *Type, ASTExpr *Size);

        ASTStringType *CreateStringType(const SourceLocation &Loc);

        ASTClassType *CreateClassType(ASTClass *Class);

        ASTClassType *CreateClassType(ASTIdentifier *Class);

        ASTEnumType *CreateEnumType(ASTEnum *Enum);

        ASTEnumType *CreateEnumType(ASTIdentifier *Enum);

        ASTIdentityType *CreateIdentityType(ASTIdentifier *Identifier);

        ASTErrorType *CreateErrorType();

        // Create Values
        ASTNullValue *CreateNullValue(const SourceLocation &Loc);

        ASTZeroValue *CreateZeroValue(const SourceLocation &Loc);

        ASTBoolValue *CreateBoolValue(const SourceLocation &Loc, bool Val);

        ASTIntegerValue *CreateIntegerValue(const SourceLocation &Loc, uint64_t Val, bool Negative = false);

        ASTIntegerValue *CreateCharValue(const SourceLocation &Loc, char Val);

        ASTFloatingValue *CreateFloatingValue(const SourceLocation &Loc, std::string Val);

        ASTFloatingValue *CreateFloatingValue(const SourceLocation &Loc, double Val);

        ASTArrayValue *CreateArrayValue(const SourceLocation &Loc, llvm::SmallVector<ASTValue *, 8> Values);

        ASTStringValue *CreateStringValue(const SourceLocation &Loc, StringRef Str);

        ASTStructValue *CreateStructValue(const SourceLocation &Loc, llvm::StringMap<ASTValue *>);

        ASTValue *CreateDefaultValue(ASTType *Type);

        ASTParam *CreateParam(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                              llvm::SmallVector<ASTScope *, 8> &Scopes, ASTValue *DefaultValue = nullptr);

        ASTParam *CreateErrorHandlerParam();

        ASTLocalVar *CreateLocalVar(ASTBlockStmt *BlockStmt, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                    llvm::SmallVector<ASTScope *, 8> &Scopes);

        // Create Identifier
        ASTIdentifier *CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        // Create Call
        ASTCall *CreateCall(ASTIdentifier *Identifier, llvm::SmallVector<ASTExpr *, 8> &Args, ASTCallKind CallKind, ASTIdentifier *Parent = nullptr);

        ASTCall *CreateCall(ASTFunction *Function, llvm::SmallVector<ASTExpr *, 8> &Args);

        ASTCall *CreateCall(ASTClassMethod *Method, ASTCallKind CallKind);

        ASTCall *CreateCall(ASTIdentifier *Instance, ASTClassMethod *Method);

        // Create VarRef
        ASTVarRef *CreateVarRef(ASTIdentifier *Identifier, ASTIdentifier *Parent = nullptr);

        ASTVarRef *CreateVarRef(ASTVar *Var);

        // Create Expressions
        ASTEmptyExpr *CreateExpr();

        ASTValueExpr *CreateExpr(ASTValue *Value);

        ASTCallExpr *CreateExpr(ASTCall *Call);

        ASTVarRefExpr *CreateExpr(ASTVarRef *VarRef);

        ASTUnaryOperatorExpr *CreateOperatorExpr(const SourceLocation &Loc, ASTUnaryOperatorKind UnaryKind);

        ASTBinaryOperatorExpr *CreateOperatorExpr(const SourceLocation &Loc, ASTBinaryOperatorKind BinaryKind);

        ASTTernaryOperatorExpr *CreateOperatorExpr(const SourceLocation &Loc, ASTTernaryOperatorKind TernaryKind);

        ASTUnaryGroupExpr *CreateUnaryExpr(ASTUnaryOperatorExpr *Operator,
                                           ASTVarRefExpr *First);

        ASTBinaryGroupExpr *CreateBinaryExpr(ASTBinaryOperatorExpr *Operator,
                                             ASTExpr *First, ASTExpr *Second);

        ASTTernaryGroupExpr *CreateTernaryExpr(ASTExpr *First,
                                               ASTTernaryOperatorExpr *FirstOperator, ASTExpr *Second,
                                               ASTTernaryOperatorExpr *SecondOperator, ASTExpr *Third);

        // Create Statements

        SemaBuilderStmt *CreateVarStmt(ASTBlockStmt *Parent, ASTVarRef *VarRef);

        SemaBuilderStmt *CreateVarStmt(ASTBlockStmt *Parent, ASTVar *Var);

        SemaBuilderStmt *CreateReturnStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

        SemaBuilderStmt *CreateFailStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTVar *ErrorHandler);

        SemaBuilderStmt *CreateExprStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

        ASTBreakStmt *CreateBreakStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

        ASTContinueStmt *CreateContinueStmt(ASTBlockStmt *Parent, const SourceLocation &Loc);

        ASTDeleteStmt *CreateDeleteStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTVarRef *VarRef);

        // Create Blocks structures

        ASTBlockStmt *CreateBody(ASTFunctionBase *FunctionBase, ASTBlockStmt *Body);

        ASTBlockStmt *CreateBlockStmt(const SourceLocation &Loc);

        SemaBuilderIfStmt *CreateIfBuilder(ASTBlockStmt *Parent);

        SemaBuilderSwitchStmt *CreateSwitchBuilder(ASTBlockStmt *Parent);

        SemaBuilderLoopStmt *CreateLoopBuilder(ASTBlockStmt *Parent, const SourceLocation &Loc);

        ASTHandleStmt *CreateHandleStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTVarRef *ErrorRef);

    };

}  // end namespace fly

#endif