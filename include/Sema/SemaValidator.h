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
#include <llvm/ADT/DenseMap.h>

#include "AST/ASTTypeRef.h"
#include "AST/ASTVar.h"

namespace fly {

    class Sema;
    class ASTBlockStmt;
    class ASTStmt;
    class ASTRef;
    class ASTModule;
    class ASTImport;
    class ASTExpr;
    class ASTVar;
    class ASTClass;
    class ASTTypeRef;
    class SourceLocation;
    class SemaModule;
    class SemaGlobalVar;
    class SemaFunction;
    class SemaClassType;
    class SemaEnumType;
    class SemaComment;
    class ASTValue;
    class SemaClassMethod;
    enum class SemaTypeKind;

    class SemaValidator {

        friend class Sema;

    public:

        static bool CheckDuplicateModules(ASTModule * Module, const llvm::DenseMap<uint64_t, SemaModule *> &Modules);

        static bool CheckDuplicateVars(const llvm::StringMap<SemaGlobalVar *> &Vars, ASTVar * Var);

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

        static bool CheckDuplicateParams(llvm::SmallVector<ASTVar *, 8> Params, ASTVar *Param);

       static bool CheckDuplicateLocalVars(ASTStmt *Stmt, llvm::StringRef VarName);

        static bool CheckCommentParams(SemaComment *Comment, const llvm::SmallVector<ASTVar*, 8> &Params);

        static bool CheckCommentReturn(SemaComment *Comment, ASTTypeRef* ReturnType);

        static bool CheckCommentFail(SemaComment *Comment);

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

        static bool CheckExpr(ASTExpr *Expr);

        static bool CheckEqualTypes(SemaType *Type1, SemaType *Type2);

        static bool CheckConvertibleTypes(SemaType *FromType, SemaType *ToType);

        static bool CheckInheritance(SemaClassType *ClassType, SemaClassType *SuperClassType);

        static bool CheckInheritance(SemaEnumType *EnumType, SemaEnumType *SuperEnumType);

        static bool CheckArithTypes(SemaType *Type1, SemaType *Type2);

        static bool CheckLogicalTypes(SemaType *Type1, SemaType *Type2);

        static bool CheckNameEmpty(const SourceLocation &Loc, llvm::StringRef Name);

        static bool CheckIsValueExpr(ASTExpr *Expr);

        static bool CheckVarRefExpr(ASTExpr *Expr);

        static bool CheckValue(ASTValue* Value);
    };

}  // end namespace fly

#endif // FLY_SEMA_VALIDATOR_H