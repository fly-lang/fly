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

    class ASTScopes;

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

    class ASTScopes;

    class ASTEnum;

    class ASTEnumEntry;

    class ASTIdentityType;

    class ASTFailStmt;

    class ASTStringValue;

    enum class ASTClassKind;
    enum class ASTUnaryOperatorKind;
    enum class ASTBinaryOperatorKind;
    enum class ASTTernaryOperatorKind;

    class SemaBuilder {

        friend class Sema;

        friend class SemaResolver;

        Sema &S;

        static std::string DEFAULT;

        ASTContext *CreateContext();

        ASTNameSpace *CreateDefaultNameSpace();

    public:

        explicit SemaBuilder(Sema &S);

        // Create Module
        ASTModule *CreateModule(const std::string &Name);

        ASTModule *CreateHeaderModule(const std::string &Name);

        // Create NameSpace
        ASTNameSpace *CreateNameSpace(ASTIdentifier *Identifier);

        // Create Top Definitions
        ASTImport *CreateImport(const SourceLocation &Loc, StringRef Name);

        ASTAlias *CreateAlias(const SourceLocation &Loc, StringRef Name);

        ASTScopes *CreateScopes(ASTVisibilityKind Visibility = ASTVisibilityKind::V_DEFAULT, bool Constant = false,
                     bool = false);

        ASTGlobalVar *CreateGlobalVar(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                        ASTScopes *Scopes);

        ASTFunction *CreateFunction(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                       ASTScopes *Scopes);

        ASTClass *CreateClass(const SourceLocation &Loc, ASTClassKind ClassKind, llvm::StringRef Name,
                              ASTScopes *Scopes, llvm::SmallVector<ASTClassType *, 4> &ClassTypes);

        ASTClassAttribute *CreateClassAttribute(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                             ASTScopes *Scopes);

        ASTClassMethod *CreateClassConstructor(const SourceLocation &Loc, ASTScopes *Scopes);

        ASTClassMethod *CreateClassMethod(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes,
                          bool Static = false);

        ASTClassMethod *CreateClassVirtualMethod(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes);

        ASTEnum *CreateEnum(const SourceLocation &Loc, llvm::StringRef Name, ASTScopes *Scopes,
                   llvm::SmallVector<ASTEnumType *, 4> EnumTypes);

        ASTEnumEntry *CreateEnumEntry(const SourceLocation &Loc, ASTEnumType *Type, llvm::StringRef Name);

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

        ASTParam *CreateParam(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes = nullptr);

        ASTParam *CreateErrorHandlerParam();

        ASTLocalVar *CreateLocalVar(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,  ASTScopes *Scopes = nullptr);

        // Create Identifier
        ASTIdentifier *CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        // Create Call
        ASTCall *CreateCall(ASTIdentifier *Identifier);

        ASTCall *CreateCall(ASTFunction *Function);

        ASTCall *CreateCall(ASTClassMethod *Method);

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

        ASTCallExpr *CreateNewExpr(ASTCall *Call);

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

        ASTFailStmt *CreateFailStmt(const SourceLocation &Loc);

        ASTExprStmt *CreateExprStmt(const SourceLocation &Loc);

        ASTDeleteStmt *CreateDeleteStmt(const SourceLocation &Loc, ASTVarRef *VarRef);

        // Create Blocks structures

        ASTBlockStmt *CreateBody(ASTFunctionBase *FunctionBase);

        ASTBlockStmt *CreateBlockStmt(const SourceLocation &Loc);

        ASTIfStmt *CreateIfStmt(const SourceLocation &Loc);

        ASTSwitchStmt *CreateSwitchStmt(const SourceLocation &Loc, ASTExpr *Expr);

        ASTLoopStmt *CreateLoopStmt(const SourceLocation &Loc, ASTExpr *Condition, ASTBlockStmt *Block);

        ASTHandleStmt *CreateHandleStmt(const SourceLocation &Loc, ASTVarRef *ErrorRef);

        /** Add AST **/

        bool AddModule(ASTModule *Module, ASTNameSpace *NameSpace = nullptr);

        // Add Module & NameSpace
        bool AddNameSpace(ASTModule *Module, ASTNameSpace *NewNameSpace, bool ExternLib = false);

        // Add Top definitions
        bool AddImport(ASTModule *Module, ASTImport *Import);

        bool AddExternalGlobalVar(ASTModule *Module, ASTGlobalVar *GlobalVar);

        bool AddExternalFunction(ASTModule *Module, ASTFunction *Function);

        bool AddExternalIdentities(ASTModule *Module, ASTIdentity *Identity);

        bool AddGlobalVar(ASTModule *Module, ASTGlobalVar *GlobalVar, ASTValue *Value = nullptr);

        bool AddFunction(ASTModule *Module, ASTFunction *Function);

        bool AddIdentity(ASTModule *Module, ASTIdentity *Identity);

        // Add details
        bool AddClassAttribute(ASTClass *Class, ASTClassAttribute *Var);

        bool AddClassMethod(ASTClass *Class, ASTClassMethod *Method);

        bool AddEnumEntry(ASTEnum *Enum, ASTEnumEntry *Entry);

        bool AddParam(ASTFunctionBase *FunctionBase, ASTParam *Param);

        void AddFunctionVarParams(ASTFunctionBase *Function, ASTParam *Param); // TODO

        bool AddComment(ASTBase *Base, llvm::StringRef Comment);

        // Add Value to Array
        bool AddArrayValue(ASTArrayValue *ArrayValue, ASTValue *Value);

        bool AddStructValue(ASTStructValue *ArrayValue, llvm::StringRef Key, ASTValue *Value);

        bool AddCallArg(ASTCall *Call, ASTExpr *Expr);

        bool AddLocalVar(ASTBlockStmt *BlockStmt, ASTLocalVar *LocalVar);

        // Add Stmt
        bool AddStmt(ASTStmt *Parent, ASTStmt *Stmt);

        bool AddElsif(ASTIfStmt *IfStmt, ASTExpr *Condition, ASTBlockStmt *Block);

        bool AddElse(ASTIfStmt *IfStmt, ASTStmt *Else);

        bool AddSwitchCase(ASTSwitchStmt *SwitchStmt, ASTValueExpr *ValueExpr, ASTBlockStmt *Block);

        bool AddSwitchDefault(ASTSwitchStmt *SwitchStmt, ASTBlockStmt *Block);

        bool AddLoopInit(ASTLoopStmt *LoopStmt, ASTBlockStmt *Block);

        bool AddLoopPost(ASTLoopStmt *LoopStmt, ASTBlockStmt *Block);

        bool AddExpr(ASTVarStmt *Stmt, ASTExpr *Expr);

        bool AddExpr(ASTExprStmt *Stmt, ASTExpr *Expr);

        bool AddExpr(ASTReturnStmt *Stmt, ASTExpr *Expr);

        bool AddExpr(ASTFailStmt *Stmt, ASTExpr *Expr);

        bool AddExpr(ASTIfStmt *Stmt, ASTExpr *Expr);

        template<class T>
        bool ContainsFunction(llvm::StringMap<std::map<uint64_t, llvm::SmallVector<T *, 4>>> &Functions,
                              T *NewFunction) {
            // Search by Name
            const auto &StrMapIt = Functions.find(NewFunction->getName());
            if (StrMapIt != Functions.end()) {

                // Search by Number of Parameters
                const auto IntMapIt = StrMapIt->second.find(NewFunction->getParams().size());

                // Search by Type of Parameters
                llvm::SmallVector<T *, 4> VectorFunctions = IntMapIt->second;
                for (auto &Function: VectorFunctions) {

                    // Check if NewFunction have no params
                    if (NewFunction->getParams().empty() && Function->getParams().empty()) {
                        return true;
                    }

                    // Check types
                    if (!S.Validator->CheckParams(Function->getParams(), NewFunction->getParams())) {
                        return true;
                    }
                }
            }
            return false;
        }

        template<class T>
        bool InsertFunction(llvm::StringMap<std::map<uint64_t, llvm::SmallVector<T *, 4>>> &Functions,
                                   T *NewFunction) {

            // Functions is llvm::StringMap<std::map <uint64_t, llvm::SmallVector <ASTFunction *, 4>>>
            const auto &StrMapIt = Functions.find(NewFunction->getName());
            if (StrMapIt == Functions.end()) { // This Module not contains a Function with this Function->Name

                // add to llvm::SmallVector
                llvm::SmallVector<T *, 4> Vect;
                Vect.push_back(NewFunction);

                // add to std::map
                std::map<uint64_t, llvm::SmallVector<T *, 4>> IntMap;
                IntMap.insert(std::make_pair(NewFunction->getParams().size(), Vect));

                // add to llvm::StringMap
                return Functions.insert(std::make_pair(NewFunction->getName(), IntMap)).second;
            } else {
                return InsertFunction(StrMapIt->second, NewFunction);
            }
        }

        template<class T>
        bool InsertFunction(std::map<uint64_t, llvm::SmallVector<T *, 4>> &Functions, T *NewFunction) {

            // This Module contains a Function with this Function->Name
            const auto &IntMapIt = Functions.find(NewFunction->getParams().size());
            if (IntMapIt == Functions.end()) { // but not have the same number of Params

                // add to llvm::SmallVector
                llvm::SmallVector<T *, 4> VectorFunctions;
                VectorFunctions.push_back(NewFunction);

                // add to std::map
                std::pair<uint64_t, SmallVector<T *, 4>> IntMapPair = std::make_pair(
                        NewFunction->getParams().size(), VectorFunctions);

                return Functions.insert(std::make_pair(NewFunction->getParams().size(), VectorFunctions)).second;
            } else { // This Module contains a Function with this Function->Name and same number of Params
                llvm::SmallVector<T *, 4> VectorFunctions = IntMapIt->second;
                for (auto &Function: VectorFunctions) {

                    // check no params duplicates
                    if (NewFunction->getParams().empty() && Function->getParams().empty()) {
                        // Error:
                        return false;
                    }

                    if (!SemaValidator::CheckParams(Function->getParams(), NewFunction->getParams())) {
                        return false;
                    }
                }

                // Add to function list
                IntMapIt->second.push_back(NewFunction);
                return true;
            }
        }
    };

}  // end namespace fly

#endif