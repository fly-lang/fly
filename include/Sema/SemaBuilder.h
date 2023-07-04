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

#include "AST/ASTClass.h"
#include "AST/ASTVar.h"
#include "Sema/Sema.h"
#include "Sema/SemaValidator.h"

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
    class ASTScopes;
    class ASTClass;
    class ASTClassVar;
    class ASTGlobalVar;
    class ASTFunction;
    class ASTFunctionBase;
    class ASTIdentifier;
    class ASTCall;
    class ASTStmt;
    class ASTBlock;
    class ASTDelete;
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
    class ASTVarAssign;
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
    class ASTBreak;
    class ASTReturn;
    class ASTContinue;
    class ASTExpr;
    class ASTEmptyExpr;
    class ASTValueExpr;
    class ASTVarRefExpr;
    class ASTCallExpr;
    class ASTUnaryGroupExpr;
    class ASTBinaryGroupExpr;
    class ASTTernaryGroupExpr;
    class ASTReference;
    class ASTScopes;
    enum class ASTUnaryOperatorKind;
    enum class ASTUnaryOptionKind;
    enum class ASTBinaryOperatorKind;

    class SemaBuilder {

        friend class Sema;
        friend class SemaResolver;

        Sema &S;

        ASTContext *Context;

    public:

        SemaBuilder(Sema &S);

        bool Build();
        void Destroy();

        // Create Node
        ASTNode *CreateNode(const std::string &Name, std::string &NameSpace);
        ASTNode *CreateHeaderNode(const std::string &Name, std::string &NameSpace);

        // Create Top Definitions
        ASTImport *CreateImport(const SourceLocation &NameLoc, StringRef Name);
        ASTImport *CreateImport(const SourceLocation &NameLoc, StringRef Name,
                                const SourceLocation &AliasLoc, StringRef Alias);
        static ASTScopes *CreateScopes(ASTVisibilityKind Visibility = ASTVisibilityKind::V_DEFAULT, bool Constant = false, bool Static = false);
        ASTGlobalVar *CreateGlobalVar(ASTNode *Node, const SourceLocation &Loc, ASTType *Type, const llvm::StringRef Name,
                                      ASTScopes *Scopes);
        ASTFunction *CreateFunction(ASTNode *Node, const SourceLocation &Loc, ASTType *Type, const llvm::StringRef Name,
                                    ASTScopes *Scopes);
        ASTClass *CreateClass(ASTNode *Node, ASTClassKind ClassKind, ASTScopes *Scopes,
                              const SourceLocation &Loc, const llvm::StringRef Name,
                              llvm::SmallVector<ASTClassType *, 4> &ClassTypes);
        ASTClass *CreateClass(ASTNode *Node, ASTClassKind ClassKind, ASTScopes *Scopes,
                              const SourceLocation &Loc, const llvm::StringRef Name);
        ASTClassVar *CreateClassVar(ASTClass *Class, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                    ASTScopes *Scopes);
        ASTClassFunction *CreateClassConstructor(ASTClass *Class, const SourceLocation &Loc, ASTScopes *Scopes);
        ASTClassFunction *CreateClassMethod(ASTClass *Class, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                            ASTScopes *Scopes);
        ASTEnum *CreateEnum(ASTNode *Node, ASTScopes *Scopes, const SourceLocation &Loc, const llvm::StringRef Name,
                            llvm::SmallVector<ASTEnumType *, 4> EnumTypes);
        ASTEnum *CreateEnum(ASTNode *Node, ASTScopes *Scopes, const SourceLocation &Loc, const llvm::StringRef Name);
        ASTEnumVar *CreateEnumVar(ASTEnum *Enum, const SourceLocation &Loc, llvm::StringRef Name, uint64_t Index);

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
        static ASTClassType *CreateClassType(ASTClass *Class);
        static ASTClassType *CreateClassType(ASTIdentifier *Class);
        static ASTEnumType *CreateEnumType(ASTEnum *Enum);
        static ASTEnumType *CreateEnumType(ASTIdentifier *Enum);

        // Create Values
        static ASTNullValue *CreateNullValue(const SourceLocation &Loc);
        static ASTZeroValue *CreateZeroValue(const SourceLocation &Loc);
        static ASTBoolValue *CreateBoolValue(const SourceLocation &Loc, bool Val);
        static ASTIntegerValue *CreateIntegerValue(const SourceLocation &Loc, uint64_t Val, bool Negative = false);
        static ASTIntegerValue *CreateCharValue(const SourceLocation &Loc, char Val);
        static ASTFloatingValue *CreateFloatingValue(const SourceLocation &Loc, std::string Val);
        static ASTFloatingValue *CreateFloatingValue(const SourceLocation &Loc, double Val);
        static ASTArrayValue *CreateArrayValue(const SourceLocation &Loc);
        static ASTStructValue *CreateStructValue(const SourceLocation &Loc);
        static ASTValue *CreateDefaultValue(ASTType *Type);

        // Create Statements
        ASTParam *CreateParam(ASTFunctionBase *Function, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, bool Constant = false);
        ASTLocalVar *CreateLocalVar(ASTBlock *Parent, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, bool Constant = false);
        ASTVarAssign *CreateVarAssign(ASTBlock *Parent, ASTVarRef *VarRef);
        ASTReturn *CreateReturn(ASTBlock *Parent, const SourceLocation &Loc);
        ASTBreak *CreateBreak(ASTBlock *Parent, const SourceLocation &Loc);
        ASTContinue *CreateContinue(ASTBlock *Parent, const SourceLocation &Loc);
        ASTExprStmt *CreateExprStmt(ASTBlock *Parent, const SourceLocation &Loc);

        // Create Identifier
        ASTIdentifier *CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        // Create Call
        ASTCall *CreateCall(ASTIdentifier *Identifier);
        ASTCall *CreateCall(ASTFunctionBase *Function);
        ASTCall *CreateCall(ASTReference *Instance, ASTFunctionBase *Function);

        // Create VarRef
        ASTVarRef *CreateVarRef(ASTIdentifier *Identifier);
        ASTVarRef *CreateVarRef(ASTVar *Var);
        ASTVarRef *CreateVarRef(ASTReference *Instance, ASTVar *Var);

        // Create Expressions
        ASTEmptyExpr *CreateExpr(ASTStmt *Stmt);
        ASTValueExpr *CreateExpr(ASTStmt *Stmt, ASTValue *Value);
        ASTCallExpr *CreateExpr(ASTStmt *Stmt, ASTCall *Call);
        ASTCallExpr *CreateNewExpr(ASTStmt *Stmt, ASTCall *Call);
        ASTDelete *CreateDelete(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *VarRef);
        ASTVarRefExpr *CreateExpr(ASTStmt *Stmt, ASTVarRef *VarRef);
        ASTUnaryGroupExpr *CreateUnaryExpr(ASTStmt *Stmt, const SourceLocation &Loc, ASTUnaryOperatorKind Kind,
                                           ASTUnaryOptionKind OptionKind, ASTVarRefExpr *First);
        ASTBinaryGroupExpr *CreateBinaryExpr(ASTStmt *Stmt, const SourceLocation &OpLoc,
                                             ASTBinaryOperatorKind Kind, ASTExpr *First, ASTExpr *Second);
        ASTTernaryGroupExpr *CreateTernaryExpr(ASTStmt *Stmt, ASTExpr *First, const SourceLocation &IfLoc,
                                               ASTExpr *Second, const SourceLocation &ElseLoc, ASTExpr *Third);

        // Create Blocks structures
        ASTBlock* CreateBody(ASTFunctionBase *FunctionBase);
        ASTBlock* CreateBlock(ASTBlock *Parent, const SourceLocation &Loc);
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

        // Add Node & NameSpace
        ASTNameSpace *AddNameSpace(const std::string &Name, bool ExternLib = false);
        bool AddNode(ASTNode *Node);

        // Add Top definitions
        bool AddImport(ASTNode *Node, ASTImport *Import);
        bool AddClass(ASTClass *Class);
        bool AddEnum(ASTEnum *Enum);
        bool AddGlobalVar(ASTGlobalVar *GlobalVar, ASTValue *Value = nullptr);
        bool AddGlobalVar(ASTGlobalVar *GlobalVar, ASTExpr *Expr);
        bool AddFunction(ASTFunction *Function);

        // Add details
        bool AddClassVar(ASTClassVar *Var);
        bool AddClassMethod(ASTClassFunction *Method);
        bool AddClassConstructor(ASTClassFunction *Constructor);
        bool AddEnumVar(ASTEnumVar *EnumVar);
        bool AddParam(ASTParam *Param);
        void AddFunctionVarParams(ASTFunction *Function, ASTParam *Param); // TODO
        bool AddComment(ASTTopDef *Top, llvm::StringRef Comment);
        bool AddComment(ASTClassVar *ClassVar, llvm::StringRef Comment);
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

        template <class T>
        bool ContainsFunction(llvm::StringMap<std::map <uint64_t,llvm::SmallVector <T *, 4>>> &Functions, T *NewFunction) {
            // Search by Name
            const auto &StrMapIt = Functions.find(NewFunction->getName());
            if (StrMapIt != Functions.end()) {

                // Search by Number of Parameters
                const auto IntMapIt = StrMapIt->second.find(NewFunction->Params->getSize());

                // Search by Type of Parameters
                llvm::SmallVector <T *, 4> VectorFunctions = IntMapIt->second;
                for (auto &F: VectorFunctions) {

                    // Check if NewFunction have no params
                    if (NewFunction->getParams()->List.empty() && F->getParams()->List.empty()) {
                        return true;
                    }

                    // Check types
                    if (!S.Validator->CheckDuplicateFunctions(VectorFunctions, NewFunction)) {
                        return true;
                    }
                }
            }
            return false;
        }

        template <class T>
        bool InsertFunction(llvm::StringMap<std::map <uint64_t,llvm::SmallVector <T *, 4>>> &Functions, T *NewFunction,
                            bool CheckDuplicate) {

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
                return InsertFunction(StrMapIt->second, NewFunction, CheckDuplicate);
            }
        }

        template <class T>
        bool InsertFunction(std::map <uint64_t,llvm::SmallVector <T *, 4>> &Functions, T *NewFunction,
                            bool CheckDuplicate) {

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
                for (auto &F: VectorFunctions) {

                    // check no params duplicates
                    if (!NewFunction->getParams()->List.empty() && !F->getParams()->List.empty()) {
                        return false;
                    }

                    if (CheckDuplicate && !S.Validator->CheckDuplicateFunctions(VectorFunctions, NewFunction)) {
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