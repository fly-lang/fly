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

    class ASTNode;

    class ASTTopDef;

    class ASTImport;

    class ASTAlias;

    class ASTScopes;

    class ASTClass;

    class ASTClassVar;

    class ASTGlobalVar;

    class ASTFunction;

    class ASTFunctionBase;

    class ASTIdentifier;

    class ASTHandleStmt;

    class ASTCall;

    class ASTStmt;

    class ASTBlock;

    class ASTDeleteStmt;

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

    class ASTUnaryGroupExpr;

    class ASTBinaryGroupExpr;

    class ASTTernaryGroupExpr;

    class ASTScopes;

    class ASTEnum;

    class ASTEnumEntry;

    class ASTIdentityType;

    class ASTError;

    class ASTFailStmt;

    class ASTStringValue;

    enum class ASTClassKind;
    enum class ASTUnaryOperatorKind;
    enum class ASTUnaryOptionKind;
    enum class ASTBinaryOperatorKind;

    class SemaBuilder {

        friend class Sema;

        friend class SemaResolver;

        Sema &S;

    public:

        explicit SemaBuilder(Sema &S);

        ASTContext *CreateContext();

        // Create Node
        ASTNode *CreateNode(const std::string &Name);

        ASTNode *CreateHeaderNode(const std::string &Name);

        // Create NameSpace
        ASTNameSpace *CreateDefaultNameSpace();

        ASTNameSpace *CreateNameSpace(ASTIdentifier *Identifier = nullptr);

        // Create Top Definitions
        ASTImport *CreateImport(const SourceLocation &Loc, StringRef Name);

        ASTAlias *CreateAlias(const SourceLocation &Loc, StringRef Name);

        ASTScopes *
        CreateScopes(ASTVisibilityKind Visibility = ASTVisibilityKind::V_DEFAULT, bool Constant = false,
                     bool = false);

        ASTGlobalVar *
        CreateGlobalVar(ASTNode *Node, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                        ASTScopes *Scopes);

        ASTFunction *
        CreateFunction(ASTNode *Node, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                       ASTScopes *Scopes);

        ASTClass *CreateClass(ASTNode *Node, ASTClassKind ClassKind, ASTScopes *Scopes,
                              const SourceLocation &Loc, llvm::StringRef Name,
                              llvm::SmallVector<ASTClassType *, 4> &ClassTypes);

        ASTClassVar *
        CreateClassVar(ASTClass *Class, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                       ASTScopes *Scopes);

        ASTClassFunction *CreateClassConstructor(ASTClass *Class, const SourceLocation &Loc, ASTScopes *Scopes);

        ASTClassFunction *
        CreateClassMethod(ASTClass *Class, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                          ASTScopes *Scopes);

        ASTEnum *
        CreateEnum(ASTNode *Node, ASTScopes *Scopes, const SourceLocation &Loc, llvm::StringRef Name,
                   llvm::SmallVector<ASTEnumType *, 4> EnumTypes);

        ASTEnum *
        CreateEnum(ASTNode *Node, ASTScopes *Scopes, const SourceLocation &Loc, llvm::StringRef Name);

        ASTEnumEntry *CreateEnumEntry(ASTEnum *Enum, const SourceLocation &Loc, llvm::StringRef Name);

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

        // Create Statements
        ASTParam *
        CreateParam(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                    ASTScopes *Scopes = nullptr);

        ASTLocalVar *
        CreateLocalVar(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,  ASTScopes *Scopes = nullptr);

        ASTVarStmt *CreateVarStmt(ASTBlock *Parent, ASTVarRef *VarRef);

        ASTVarStmt *CreateVarStmt(ASTBlock *Parent, ASTVar *Var);

        ASTReturnStmt *CreateReturn(ASTBlock *Parent, const SourceLocation &Loc);

        ASTBreakStmt *CreateBreak(ASTBlock *Parent, const SourceLocation &Loc);

        ASTContinueStmt *CreateContinue(ASTBlock *Parent, const SourceLocation &Loc);

        ASTFailStmt *CreateFail(ASTBlock *Block, const SourceLocation &Loc);

        ASTExprStmt *CreateExprStmt(ASTBlock *Parent, const SourceLocation &Loc);

        // Create Identifier
        ASTIdentifier *CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        // Create Call
        ASTCall *CreateCall(ASTIdentifier *Identifier);

        ASTCall *CreateCall(ASTFunctionBase *Function);

        ASTCall *CreateCall(ASTIdentifier *Instance, ASTFunctionBase *Function);

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

        ASTDeleteStmt *CreateDelete(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *VarRef);

        ASTUnaryGroupExpr *CreateUnaryExpr(const SourceLocation &Loc, ASTUnaryOperatorKind Kind,
                                           ASTUnaryOptionKind OptionKind, ASTVarRefExpr *First);

        ASTBinaryGroupExpr *CreateBinaryExpr(const SourceLocation &OpLoc,
                                             ASTBinaryOperatorKind Kind, ASTExpr *First, ASTExpr *Second);

        ASTTernaryGroupExpr *CreateTernaryExpr(ASTExpr *First, const SourceLocation &IfLoc,
                                               ASTExpr *Second, const SourceLocation &ElseLoc, ASTExpr *Third);

        // Create Blocks structures
        ASTBlock *CreateBody(ASTFunctionBase *FunctionBase);

        ASTBlock *CreateBlock(ASTBlock *Parent, const SourceLocation &Loc);

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

        ASTHandleStmt *CreateHandleStmt(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *ErrorRef);

        /** Add AST **/

        // Add Node & NameSpace
        bool AddNameSpace(ASTNameSpace *NewNameSpace, ASTNode *Node = nullptr, bool ExternLib = false);

        bool AddNode(ASTNode *Node);

        // Add Top definitions
        bool AddImport(ASTNode *Node, ASTImport *Import);

        bool AddIdentity(ASTIdentity *Identity);

        bool AddGlobalVar(ASTGlobalVar *GlobalVar, ASTValue *Value = nullptr);

        bool AddFunction(ASTFunction *Function);

        // Add details
        bool AddClassVar(ASTClassVar *Var);

        bool AddClassMethod(ASTClassFunction *Method);

        bool AddClassConstructor(ASTClassFunction *Constructor);

        bool AddEnumEntry(ASTEnumEntry *EnumVar);

        bool AddParam(ASTFunctionBase *FunctionBase, ASTParam *Param);

        void AddFunctionVarParams(ASTFunction *Function, ASTParam *Param); // TODO

        bool AddComment(ASTBase *Base, llvm::StringRef Comment);

        bool AddComment(ASTClassFunction *ClassFunction, llvm::StringRef Comment);

        bool AddExternalGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar);

        bool AddExternalFunction(ASTNode *Node, ASTFunction *Function);

        // Add Value to Array
        bool AddArrayValue(ASTArrayValue *ArrayValue, ASTValue *Value);

        bool AddStructValue(ASTStructValue *ArrayValue, llvm::StringRef Key, ASTValue *Value);

        bool AddCallArg(ASTCall *Call, ASTExpr *Expr);

        // Add Stmt
        bool AddStmt(ASTStmt *Stmt);

        bool AddBlock(ASTBlock *Block);

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
            if (StrMapIt == Functions.end()) { // This Node not contains a Function with this Function->Name

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

            // This Node contains a Function with this Function->Name
            const auto &IntMapIt = Functions.find(NewFunction->getParams().size());
            if (IntMapIt == Functions.end()) { // but not have the same number of Params

                // add to llvm::SmallVector
                llvm::SmallVector<T *, 4> VectorFunctions;
                VectorFunctions.push_back(NewFunction);

                // add to std::map
                std::pair<uint64_t, SmallVector<T *, 4>> IntMapPair = std::make_pair(
                        NewFunction->getParams().size(), VectorFunctions);

                return Functions.insert(std::make_pair(NewFunction->getParams().size(), VectorFunctions)).second;
            } else { // This Node contains a Function with this Function->Name and same number of Params
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

        bool AddExpr(ASTStmt *Stmt, ASTExpr *Expr);
    };

}  // end namespace fly

#endif