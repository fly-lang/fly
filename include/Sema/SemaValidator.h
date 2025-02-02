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

#include "AST/ASTType.h"
#include "AST/ASTVar.h"
#include "map"

namespace fly {

    class Sema;
    class ASTBlockStmt;
    class ASTStmt;
    class ASTLocalVar;
    class ASTVarRef;
    class ASTModule;
    class ASTImport;
    class ASTExpr;
    class ASTVar;
    class ASTType;
    class ASTClass;
    class ASTTypeRef;
    class SourceLocation;
    class SymGlobalVar;
    class SymFunction;
    class SymClass;
    class SymEnum;
    enum class ASTClassKind;

    class SemaValidator {

        friend class Sema;

        Sema &S;

        SemaValidator(Sema &S);

    public:

        bool DiagEnabled = true;

        bool CheckDuplicateModules(ASTModule * Module);

        bool CheckDuplicateVars(const llvm::StringMap<SymGlobalVar *> &Vars, ASTVar * Var);

        bool CheckDuplicateFunctions(const llvm::SmallVector<SymFunction *, 8> & Functions, ASTFunction * function);

        // bool CheckDuplicateIdentities(const llvm::StringMap<SymIdentity *> &Identities, ASTIdentity * Identity);

        bool CheckClass(ASTClass* Class);

        bool CheckEnum(ASTEnum* Enum);

        template<class T>
        static bool InsertFunction(llvm::StringMap<std::map<uint64_t, llvm::SmallVector<T *, 4>>> &Functions,
                            T *NewFunction) {

            // Functions is llvm::StringMap<std::map <uint64_t, llvm::SmallVector <ASTFunction *, 4>>>
            const auto &StrMapIt = Functions.find(NewFunction->getName());
            if (StrMapIt == Functions.end()) { // Functions not contains a Function with this Function->Name

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
        static bool InsertFunction(std::map<uint64_t, llvm::SmallVector<T *, 4>> &Functions, T *NewFunction) {

            // This Module contains a Function with this Function->Name
            const auto &IntMapIt = Functions.find(NewFunction->getParams().size());
            if (IntMapIt == Functions.end()) { // but not have the same number of Params

                // add to llvm::SmallVector
                llvm::SmallVector<T *, 4> VectorFunctions;
                VectorFunctions.push_back(NewFunction);

                // add to std::map
                std::pair<uint64_t, llvm::SmallVector<T *, 4>> IntMapPair = std::make_pair(
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

//                    if (!SemaValidator::CheckParams(Function->getParams(), NewFunction->getParams())) {
//                        return false;
//                    }
                }

                // Add to function list
                IntMapIt->second.push_back(NewFunction);
                return true;
            }
        }

        bool CheckDuplicateParams(llvm::SmallVector<ASTVar *, 8> Params, ASTVar *Param);

        bool CheckDuplicateLocalVars(ASTStmt *Stmt, llvm::StringRef VarName);

        bool CheckCommentParams(const llvm::SmallVector<ASTVar*, 8> &Params);

        bool CheckCommentReturn(ASTType* ReturnType);

        static bool CheckParams(llvm::SmallVector<ASTVar *, 8> Params, llvm::SmallVector<ASTVar *, 8> CheckParams) {
            // Types will be checked on Resolve()
            for (ASTVar *Param : Params) {
                for (ASTVar *CheckParam : CheckParams) {
                    if (CheckEqualTypes(Param->getTypeRef(), CheckParam->getTypeRef())) {
                        return false;
                    }
                }
            }
            return true;
        }

        bool CheckImport(ASTImport *Import);

        bool CheckExpr(ASTExpr *Expr);

        static bool CheckEqualTypes(ASTType *Type1, ASTType *Type2);

        bool CheckEqualTypes(ASTType *Type, ASTTypeKind Kind);

        bool CheckConvertibleTypes(ASTType *FromType, ASTType *ToType);

        bool CheckArithTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2);

        bool CheckLogicalTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2);

        // static bool CheckClassInheritance(fly::ASTClassType *FromType, fly::ASTClassType *ToType);

        void CheckCreateModule(const std::string &Name);

        void CheckCreateNameSpace(const SourceLocation &Loc, llvm::StringRef Name);

        void CheckCreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        void CheckCreateImport(const SourceLocation &Loc, StringRef Name);

        void CheckCreateAlias(const SourceLocation &Loc, StringRef Name);

        void CheckCreateGlobalVar(const SourceLocation &Loc, ASTTypeRef *TypeRef, StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes);

        void CheckCreateFunction(const SourceLocation &Loc, ASTTypeRef *TypeRef, StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes);

        void CheckCreateClass(const SourceLocation &Loc, StringRef Name, ASTClassKind ClassKind,
                              llvm::SmallVector<ASTScope *, 8> &Scopes,
                              SmallVector<ASTClassType *, 4> &ClassTypes);

        void CheckCreateClassAttribute(const SourceLocation &Loc, StringRef Name, ASTTypeRef *TypeRef, llvm::SmallVector<ASTScope *, 8> &Scopes);

        void CheckCreateClassConstructor(const SourceLocation &Loc, llvm::SmallVector<ASTScope *, 8> &Scopes);

        void CheckCreateClassMethod(const SourceLocation &Loc, ASTTypeRef *TypeRef, StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes);

        void CheckCreateEnum(const SourceLocation &Loc, const StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                             SmallVector<ASTEnumType *, 4> EnumTypes);

        void CheckCreateEnumEntry(const SourceLocation &Loc, StringRef Name);

        void CheckCreateParam(const SourceLocation &Loc, ASTTypeRef *TypeRef, StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes);

        void CheckCreateLocalVar(const SourceLocation &Loc, ASTTypeRef *TypeRef, StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes);

        bool CheckValueExpr(ASTExpr *Expr);

        bool CheckVarRefExpr(ASTExpr *Expr);

        bool CheckScopes(const SmallVector<ASTScope *, 8> &vector);
    };

}  // end namespace fly

#endif // FLY_SEMA_VALIDATOR_H