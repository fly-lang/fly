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

#include "AST/ASTType.h"
#include "AST/ASTParam.h"
#include "AST/ASTClassType.h"

namespace fly {

    class Sema;
    class ASTGlobalVar;
    class ASTBlockStmt;
    class ASTStmt;
    class ASTLocalVar;
    class ASTVarRef;
    class ASTModule;
    class ASTImport;
    class ASTExpr;
    class ASTParam;
    class ASTType;
    class SourceLocation;
    class ASTEnumType;
    enum class ASTClassKind;

    class SemaValidator {

        friend class Sema;

        Sema &S;

        SemaValidator(Sema &S);

    public:

        bool DiagEnabled = true;

        bool CheckDuplicateParams(llvm::SmallVector<ASTParam *, 8> Params, ASTParam *Param);

        bool CheckDuplicateLocalVars(ASTStmt *Stmt, llvm::StringRef VarName);

        static bool CheckParams(llvm::SmallVector<ASTParam *, 8> Params, llvm::SmallVector<ASTParam *, 8> CheckParams) {
            // Types will be checked on Resolve()
            for (ASTParam *Param : Params) {
                for (ASTParam *CheckParam : CheckParams) {
                    if (CheckEqualTypes(Param->getType(), CheckParam->getType())) {
                        return false;
                    }
                }
            }
            return true;
        }

        bool CheckImport(ASTModule *Module, ASTImport *Import);

        bool CheckExpr(ASTExpr *Expr);

        static bool CheckEqualTypes(ASTType *Type1, ASTType *Type2);

        bool CheckEqualTypes(ASTType *Type, ASTTypeKind Kind);

        bool CheckConvertibleTypes(ASTType *FromType, ASTType *ToType);

        bool CheckArithTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2);

        bool CheckLogicalTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2);

        static bool CheckClassInheritance(fly::ASTClassType *FromType, fly::ASTClassType *ToType);

        void CheckCreateModule(const std::string &Name);

        void CheckCreateNameSpace(const SourceLocation &Loc, llvm::StringRef Name);

        void CheckCreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        void CheckCreateImport(const SourceLocation &Loc, StringRef Name);

        void CheckCreateAlias(const SourceLocation &Loc, StringRef Name);

        void CheckCreateGlobalVar(const SourceLocation &Loc, ASTType *Type, StringRef Name, ASTScopes *Scopes);

        void CheckCreateFunction(const SourceLocation &Loc, ASTType *Type, StringRef Name, ASTScopes *Scopes);

        void CheckCreateClass(const SourceLocation &Loc, StringRef Name, ASTClassKind ClassKind,
                              ASTScopes *Scopes,
                              SmallVector<ASTClassType *, 4> &ClassTypes);

        void CheckCreateClassVar(const SourceLocation &Loc, StringRef Name, ASTType *Type, ASTScopes *Scopes);

        void CheckCreateClassConstructor(const SourceLocation &Loc, ASTScopes *Scopes);

        void CheckCreateClassMethod(const SourceLocation &Loc, ASTType *Type, StringRef Name, ASTScopes *Scopes);

        void CheckCreateEnum(const SourceLocation &Loc, const StringRef Name, ASTScopes *Scopes,
                             SmallVector<ASTEnumType *, 4> EnumTypes);

        void CheckCreateEnumEntry(const SourceLocation &Loc, StringRef Name);

        void CheckCreateParam(const SourceLocation &Loc, ASTType *Type, StringRef Name, ASTScopes *Scopes);

        void CheckCreateLocalVar(const SourceLocation &Loc, ASTType *Type, StringRef Name, ASTScopes *Scopes);

        bool CheckVarRefExpr(ASTExpr *Expr);

        bool CheckValue(ASTValue *Value);
    };

}  // end namespace fly

#endif // FLY_SEMA_VALIDATOR_H