//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Sema.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_VALIDATOR_H
#define FLY_SEMA_VALIDATOR_H

#include <AST/ASTEnum.h>

#include "AST/ASTTypeRef.h"
#include "AST/ASTVar.h"
#include "map"

namespace fly {

    class Sema;
    class ASTBlockStmt;
    class ASTStmt;
    class ASTVarRef;
    class ASTModule;
    class ASTImport;
    class ASTExpr;
    class ASTVar;
    class ASTClass;
    class ASTTypeRef;
    class SourceLocation;
    class SymGlobalVar;
    class SymFunction;
    class SymClass;
    class SymEnum;
    class SymComment;
    enum class SymTypeKind;

    class SemaValidator {

        friend class Sema;

        Sema &S;

        SemaValidator(Sema &S);

    public:

        bool DiagEnabled = true;

        bool CheckDuplicateModules(ASTModule * Module);

        bool CheckDuplicateVars(const llvm::StringMap<SymGlobalVar *> &Vars, ASTVar * Var);

//         template<class T>
//         static bool InsertFunction(llvm::StringMap<std::map<uint64_t, llvm::SmallVector<T *, 4>>> &Functions,
//                             T *NewFunction) {
//
//             // Functions is llvm::StringMap<std::map <uint64_t, llvm::SmallVector <ASTFunction *, 4>>>
//             const auto &StrMapIt = Functions.find(NewFunction->getName());
//             if (StrMapIt == Functions.end()) { // Functions not contains a Function with this Function->Name
//
//                 // add to llvm::SmallVector
//                 llvm::SmallVector<T *, 4> Vect;
//                 Vect.push_back(NewFunction);
//
//                 // add to std::map
//                 std::map<uint64_t, llvm::SmallVector<T *, 4>> IntMap;
//                 IntMap.insert(std::make_pair(NewFunction->getParams().size(), Vect));
//
//                 // add to llvm::StringMap
//                 return Functions.insert(std::make_pair(NewFunction->getName(), IntMap)).second;
//             } else {
//                 return InsertFunction(StrMapIt->second, NewFunction);
//             }
//         }
//
//         template<class T>
//         static bool InsertFunction(std::map<uint64_t, llvm::SmallVector<T *, 4>> &Functions, T *NewFunction) {
//
//             // This Module contains a Function with this Function->Name
//             const auto &IntMapIt = Functions.find(NewFunction->getParams().size());
//             if (IntMapIt == Functions.end()) { // but not have the same number of Params
//
//                 // add to llvm::SmallVector
//                 llvm::SmallVector<T *, 4> VectorFunctions;
//                 VectorFunctions.push_back(NewFunction);
//
//                 // add to std::map
//                 std::pair<uint64_t, llvm::SmallVector<T *, 4>> IntMapPair = std::make_pair(
//                         NewFunction->getParams().size(), VectorFunctions);
//
//                 return Functions.insert(std::make_pair(NewFunction->getParams().size(), VectorFunctions)).second;
//             } else { // This Module contains a Function with this Function->Name and same number of Params
//                 llvm::SmallVector<T *, 4> VectorFunctions = IntMapIt->second;
//                 for (auto &Function: VectorFunctions) {
//
//                     // check no params duplicates
//                     if (NewFunction->getParams().empty() && Function->getParams().empty()) {
//                         // Error:
//                         return false;
//                     }
//
// //                    if (!SemaValidator::CheckParams(Function->getParams(), NewFunction->getParams())) {
// //                        return false;
// //                    }
//                 }
//
//                 // Add to function list
//                 IntMapIt->second.push_back(NewFunction);
//                 return true;
//             }
//         }

        bool CheckDuplicateParams(llvm::SmallVector<ASTVar *, 8> Params, ASTVar *Param);

        bool CheckDuplicateLocalVars(ASTStmt *Stmt, llvm::StringRef VarName);

        bool CheckCommentParams(SymComment *Comment, const llvm::SmallVector<ASTVar*, 8> &Params);

        bool CheckCommentReturn(SymComment *Comment, ASTTypeRef* ReturnType);

        bool CheckCommentFail(SymComment *Comment);

        // static bool CheckParams(llvm::SmallVector<ASTVar *, 8> Params, llvm::SmallVector<ASTVar *, 8> CheckParams) {
        //     // Types will be checked on Resolve()
        //     for (ASTVar *Param : Params) {
        //         for (ASTVar *CheckParam : CheckParams) {
        //             if (CheckEqualTypes(Param->getTypeRef(), CheckParam->getTypeRef())) {
        //                 return false;
        //             }
        //         }
        //     }
        //     return true;
        // }

        bool CheckExpr(ASTExpr *Expr);

        bool CheckEqualTypes(SymType *Type1, SymType *Type2);

        bool CheckConvertibleTypes(SymType *FromType, SymType *ToType);

        bool CheckInheritance(SymClass *TheClass, SymClass *SuperClass);

        bool CheckInheritance(SymEnum *TheEnum, SymEnum *SuperEnum);

        bool CheckArithTypes(SymType *Type1, SymType *Type2);

        bool CheckLogicalTypes(SymType *Type1, SymType *Type2);

        void CheckNameEmpty(const SourceLocation &Loc, llvm::StringRef Name);

        bool CheckIsValueExpr(ASTExpr *Expr);

        bool CheckVarRefExpr(ASTExpr *Expr);

        bool CheckValue(ASTValue* Value);
    };

}  // end namespace fly

#endif // FLY_SEMA_VALIDATOR_H