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
                                    SmallVector<ASTScope *, 8> &Scopes);

        ASTClass *CreateClass(ASTModule *Module, const SourceLocation &Loc, ASTClassKind ClassKind, llvm::StringRef Name,
                              SmallVector<ASTScope *, 8> &Scopes, llvm::SmallVector<ASTClassType *, 4> &ClassTypes);

        ASTClassAttribute *CreateClassAttribute(const SourceLocation &Loc, ASTClass &Class, ASTType *Type, llvm::StringRef Name,
                                                SmallVector<ASTScope *, 8> &Scopes);

        ASTClassMethod *CreateClassConstructor(const SourceLocation &Loc, ASTClass &Class,
                                               llvm::SmallVector<ASTScope *, 8> &Scopes);

        ASTClassMethod *CreateClassMethod(const SourceLocation &Loc, ASTClass &Class, ASTType *Type,
                                          llvm::StringRef Name, SmallVector<ASTScope *, 8> &Scopes);

        ASTClassMethod *CreateClassVirtualMethod(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                                 SmallVector<ASTScope *, 8> &Scopes);

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

        ASTArrayValue *CreateArrayValue(const SourceLocation &Loc);

        ASTStringValue *CreateStringValue(const SourceLocation &Loc, StringRef Str);

        ASTStructValue *CreateStructValue(const SourceLocation &Loc);

        ASTValue *CreateDefaultValue(ASTType *Type);

        ASTParam *CreateParam(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                              llvm::SmallVector<ASTScope *, 8> &Scopes);

        ASTParam *CreateErrorHandlerParam();

        ASTLocalVar *CreateLocalVar(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                    llvm::SmallVector<ASTScope *, 8> &Scopes);

        // Create Identifier
        ASTIdentifier *CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        // Create Call
        ASTCall *CreateCall(ASTIdentifier *Identifier, ASTCallKind CallKind);

        ASTCall *CreateCall(ASTFunction *Function);

        ASTCall *CreateCall(ASTClassMethod *Method, ASTCallKind CallKind);

        ASTCall *CreateCall(ASTIdentifier *Instance, ASTClassMethod *Method);

        ASTArg *CreateArg(ASTExpr *Expr);

        // Create VarRef
        ASTVarRef *CreateVarRef(ASTIdentifier *Identifier);

        ASTVarRef *CreateVarRef(ASTVar *Var);

        ASTVarRef *CreateVarRef(ASTIdentifier *Instance, ASTVar *Var);

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

        ASTVarStmt *CreateVarStmt(ASTVarRef *VarRef);

        ASTVarStmt *CreateVarStmt(ASTVar *Var);

        ASTReturnStmt *CreateReturnStmt(const SourceLocation &Loc);

        ASTBreakStmt *CreateBreakStmt(const SourceLocation &Loc);

        ASTContinueStmt *CreateContinueStmt(const SourceLocation &Loc);

        ASTFailStmt *CreateFailStmt(const SourceLocation &Loc, ASTVar *ErrorHandler);

        ASTExprStmt *CreateExprStmt(const SourceLocation &Loc);

        ASTDeleteStmt *CreateDeleteStmt(const SourceLocation &Loc, ASTVarRef *VarRef);

        // Create Blocks structures

        ASTBlockStmt *CreateBody(ASTFunctionBase *FunctionBase);

        ASTBlockStmt *CreateBlockStmt(const SourceLocation &Loc);

        ASTIfStmt *CreateIfStmt(const SourceLocation &Loc);

        ASTSwitchStmt *CreateSwitchStmt(const SourceLocation &Loc);

        ASTLoopStmt *CreateLoopStmt(const SourceLocation &Loc);

        ASTHandleStmt *CreateHandleStmt(const SourceLocation &Loc, ASTVarRef *ErrorRef);

        /** Add AST **/

        bool AddParam(ASTFunctionBase *FunctionBase, ASTParam *Param);

        void AddFunctionVarParams(ASTFunctionBase *Function, ASTParam *Param); // TODO

        // Add Value to Array
        bool AddArrayValue(ASTArrayValue *ArrayValue, ASTValue *Value);

        bool AddStructValue(ASTStructValue *ArrayValue, llvm::StringRef Key, ASTValue *Value);

        bool AddCallArg(ASTCall *Call, ASTExpr *Expr);

        bool AddLocalVar(ASTBlockStmt *BlockStmt, ASTLocalVar *LocalVar);

        // Add Stmt
        bool AddStmt(ASTStmt *Parent, ASTStmt *Stmt);

        bool AddElsif(ASTIfStmt *IfStmt, ASTExpr *Condition, ASTStmt *Stmt);

        bool AddElse(ASTIfStmt *IfStmt, ASTStmt *Else);

        bool AddSwitchCase(ASTSwitchStmt *SwitchStmt, ASTValueExpr *ValueExpr, ASTStmt *Stmt);

        bool AddSwitchDefault(ASTSwitchStmt *SwitchStmt, ASTStmt *Stmt);

        bool AddLoopInit(ASTLoopStmt *LoopStmt, ASTStmt *Stmt);

        bool AddLoopPost(ASTLoopStmt *LoopStmt, ASTStmt *Stmt);

        bool AddExpr(ASTVarStmt *Stmt, ASTExpr *Expr);

        bool AddExpr(ASTExprStmt *Stmt, ASTExpr *Expr);

        bool AddExpr(ASTReturnStmt *Stmt, ASTExpr *Expr);

        bool AddExpr(ASTFailStmt *Stmt, ASTExpr *Expr);

        bool AddExpr(ASTIfStmt *Stmt, ASTExpr *Expr);

        bool AddExpr(ASTSwitchStmt *SwitchStmt, ASTExpr *Expr);

        bool AddExpr(ASTLoopStmt *LoopStmt, ASTExpr *Expr);
    };

}  // end namespace fly

#endif