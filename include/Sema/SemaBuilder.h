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

        SemaBuilder(Sema &S);

        ASTContext *CreateContext();

        // Create Node
        ASTNode *CreateNode(const std::string &Name);

        ASTNode *CreateHeaderNode(const std::string &Name);

        // Create NameSpace
        ASTNameSpace *CreateDefaultNameSpace();

        ASTNameSpace *CreateNameSpace(ASTIdentifier *Identifier = nullptr);

        // Create Top Definitions
        static ASTImport *CreateImport(const SourceLocation &Loc, StringRef Name);

        static ASTAlias *CreateAlias(const SourceLocation &Loc, StringRef Name);

        static ASTScopes *
        CreateScopes(ASTVisibilityKind Visibility = ASTVisibilityKind::V_DEFAULT, bool Constant = false,
                     bool Static = false);

        static ASTGlobalVar *
        CreateGlobalVar(ASTNode *Node, const SourceLocation &Loc, ASTType *Type, const llvm::StringRef Name,
                        ASTScopes *Scopes);

        static ASTFunction *
        CreateFunction(ASTNode *Node, const SourceLocation &Loc, ASTType *Type, const llvm::StringRef Name,
                       ASTScopes *Scopes);

        ASTClass *CreateClass(ASTNode *Node, ASTClassKind ClassKind, ASTScopes *Scopes,
                              const SourceLocation &Loc, const llvm::StringRef Name,
                              llvm::SmallVector<ASTClassType *, 4> &ClassTypes);

        static ASTClassVar *
        CreateClassVar(ASTClass *Class, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                       ASTScopes *Scopes);

        static ASTClassFunction *CreateClassConstructor(ASTClass *Class, const SourceLocation &Loc, ASTScopes *Scopes);

        static ASTClassFunction *
        CreateClassMethod(ASTClass *Class, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                          ASTScopes *Scopes);

        static ASTEnum *
        CreateEnum(ASTNode *Node, ASTScopes *Scopes, const SourceLocation &Loc, const llvm::StringRef Name,
                   llvm::SmallVector<ASTEnumType *, 4> EnumTypes);

        static ASTEnum *
        CreateEnum(ASTNode *Node, ASTScopes *Scopes, const SourceLocation &Loc, const llvm::StringRef Name);

        static ASTEnumEntry *CreateEnumEntry(ASTEnum *Enum, const SourceLocation &Loc, llvm::StringRef Name);

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

        static ASTStringType *CreateStringType(const SourceLocation &Loc);

        static ASTClassType *CreateClassType(ASTClass *Class);

        static ASTClassType *CreateClassType(ASTIdentifier *Class);

        static ASTEnumType *CreateEnumType(ASTEnum *Enum);

        static ASTEnumType *CreateEnumType(ASTIdentifier *Enum);

        static ASTIdentityType *CreateIdentityType(ASTIdentifier *Identifier);

        static ASTErrorType *CreateErrorType();

        // Create Values
        static ASTNullValue *CreateNullValue(const SourceLocation &Loc);

        static ASTZeroValue *CreateZeroValue(const SourceLocation &Loc);

        static ASTBoolValue *CreateBoolValue(const SourceLocation &Loc, bool Val);

        static ASTIntegerValue *CreateIntegerValue(const SourceLocation &Loc, uint64_t Val, bool Negative = false);

        static ASTIntegerValue *CreateCharValue(const SourceLocation &Loc, char Val);

        static ASTFloatingValue *CreateFloatingValue(const SourceLocation &Loc, std::string Val);

        static ASTFloatingValue *CreateFloatingValue(const SourceLocation &Loc, double Val);

        static ASTArrayValue *CreateArrayValue(const SourceLocation &Loc);

        static ASTStringValue *CreateStringValue(const SourceLocation &Loc, StringRef Str);

        static ASTStructValue *CreateStructValue(const SourceLocation &Loc);

        static ASTValue *CreateDefaultValue(ASTType *Type);

        // Create Statements
        static ASTParam *
        CreateParam(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                    ASTScopes *Scopes = nullptr);

        static ASTLocalVar *
        CreateLocalVar(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,  ASTScopes *Scopes = nullptr);

        static ASTVarStmt *CreateVarStmt(ASTBlock *Parent, ASTVarRef *VarRef);

        static ASTVarStmt *CreateVarStmt(ASTBlock *Parent, ASTVar *Var);

        static ASTReturnStmt *CreateReturn(ASTBlock *Parent, const SourceLocation &Loc);

        static ASTBreakStmt *CreateBreak(ASTBlock *Parent, const SourceLocation &Loc);

        static ASTContinueStmt *CreateContinue(ASTBlock *Parent, const SourceLocation &Loc);

        static ASTFailStmt *CreateFail(ASTBlock *Block, const SourceLocation &Loc);

        static ASTExprStmt *CreateExprStmt(ASTBlock *Parent, const SourceLocation &Loc);

        // Create Identifier
        static ASTIdentifier *CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        // Create Call
        static ASTCall *CreateCall(ASTIdentifier *Identifier);

        static ASTCall *CreateCall(ASTFunctionBase *Function);

        static ASTCall *CreateCall(ASTIdentifier *Instance, ASTFunctionBase *Function);

        // Create VarRef
        static ASTVarRef *CreateVarRef(ASTIdentifier *Identifier);

        static ASTVarRef *CreateVarRef(ASTVar *Var);

        static ASTVarRef *CreateVarRef(ASTIdentifier *Instance, ASTVar *Var);

        // Create Expressions
        ASTEmptyExpr *CreateExpr();

        ASTValueExpr *CreateExpr(ASTValue *Value);

        ASTCallExpr *CreateExpr(ASTCall *Call);

        ASTVarRefExpr *CreateExpr(ASTVarRef *VarRef);

        ASTCallExpr *CreateNewExpr(ASTCall *Call);

        static ASTDeleteStmt *CreateDelete(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *VarRef);

        ASTUnaryGroupExpr *CreateUnaryExpr(const SourceLocation &Loc, ASTUnaryOperatorKind Kind,
                                           ASTUnaryOptionKind OptionKind, ASTVarRefExpr *First);

        ASTBinaryGroupExpr *CreateBinaryExpr(const SourceLocation &OpLoc,
                                             ASTBinaryOperatorKind Kind, ASTExpr *First, ASTExpr *Second);

        ASTTernaryGroupExpr *CreateTernaryExpr(ASTExpr *First, const SourceLocation &IfLoc,
                                               ASTExpr *Second, const SourceLocation &ElseLoc, ASTExpr *Third);

        // Create Blocks structures
        static ASTBlock *CreateBody(ASTFunctionBase *FunctionBase);

        static ASTBlock *CreateBlock(ASTBlock *Parent, const SourceLocation &Loc);

        static ASTIfBlock *CreateIfBlock(ASTBlock *Parent, const SourceLocation &Loc);

        static ASTElsifBlock *CreateElsifBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc);

        static ASTElseBlock *CreateElseBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc);

        static ASTSwitchBlock *CreateSwitchBlock(ASTBlock *Parent, const SourceLocation &Loc);

        static ASTSwitchCaseBlock *CreateSwitchCaseBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc);

        static ASTSwitchDefaultBlock *CreateSwitchDefaultBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc);

        static ASTWhileBlock *CreateWhileBlock(ASTBlock *Parent, const SourceLocation &Loc);

        static ASTForBlock *CreateForBlock(ASTBlock *Parent, const SourceLocation &Loc);

        static ASTForLoopBlock *CreateForLoopBlock(ASTForBlock *Parent, const SourceLocation &Loc);

        static ASTForPostBlock *CreateForPostBlock(ASTForBlock *Parent, const SourceLocation &Loc);

        static ASTHandleStmt *CreateHandleStmt(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *ErrorRef);

        /** Add AST **/

        // Add Node & NameSpace
        bool AddNameSpace(ASTNameSpace *NewNameSpace, ASTNode *Node = nullptr, bool ExternLib = false);

        bool AddNode(ASTNode *Node);

        // Add Top definitions
        bool AddImport(ASTNode *Node, ASTImport *Import);

        bool AddIdentity(ASTIdentity *Identity);

        bool AddGlobalVar(ASTGlobalVar *GlobalVar, ASTValue *Value = nullptr);

        static bool AddFunction(ASTFunction *Function);

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
        static bool AddArrayValue(ASTArrayValue *ArrayValue, ASTValue *Value);

        static bool AddStructValue(ASTStructValue *ArrayValue, llvm::StringRef Key, ASTValue *Value);

        static bool AddCallArg(ASTCall *Call, ASTExpr *Expr);

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
                const auto IntMapIt = StrMapIt->second.find(NewFunction->Params->getSize());

                // Search by Type of Parameters
                llvm::SmallVector<T *, 4> VectorFunctions = IntMapIt->second;
                for (auto &Function: VectorFunctions) {

                    // Check if NewFunction have no params
                    if (NewFunction->getParams()->isEmpty() && Function->getParams()->isEmpty()) {
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
        static bool InsertFunction(llvm::StringMap<std::map<uint64_t, llvm::SmallVector<T *, 4>>> &Functions,
                                   T *NewFunction) {

            // Functions is llvm::StringMap<std::map <uint64_t, llvm::SmallVector <ASTFunction *, 4>>>
            const auto &StrMapIt = Functions.find(NewFunction->getName());
            if (StrMapIt == Functions.end()) { // This Node not contains a Function with this Function->Name

                // add to llvm::SmallVector
                llvm::SmallVector<T *, 4> Vect;
                Vect.push_back(NewFunction);

                // add to std::map
                std::map<uint64_t, llvm::SmallVector<T *, 4>> IntMap;
                IntMap.insert(std::make_pair(NewFunction->Params->getSize(), Vect));

                // add to llvm::StringMap
                return Functions.insert(std::make_pair(NewFunction->getName(), IntMap)).second;
            } else {
                return InsertFunction(StrMapIt->second, NewFunction);
            }
        }

        template<class T>
        static bool InsertFunction(std::map<uint64_t, llvm::SmallVector<T *, 4>> &Functions, T *NewFunction) {

            // This Node contains a Function with this Function->Name
            const auto &IntMapIt = Functions.find(NewFunction->Params->getSize());
            if (IntMapIt == Functions.end()) { // but not have the same number of Params

                // add to llvm::SmallVector
                llvm::SmallVector<T *, 4> VectorFunctions;
                VectorFunctions.push_back(NewFunction);

                // add to std::map
                std::pair<uint64_t, SmallVector<T *, 4>> IntMapPair = std::make_pair(
                        NewFunction->Params->getSize(), VectorFunctions);

                return Functions.insert(std::make_pair(NewFunction->Params->getSize(), VectorFunctions)).second;
            } else { // This Node contains a Function with this Function->Name and same number of Params
                llvm::SmallVector<T *, 4> VectorFunctions = IntMapIt->second;
                for (auto &Function: VectorFunctions) {

                    // check no params duplicates
                    if (NewFunction->getParams()->isEmpty() && Function->getParams()->isEmpty()) {
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