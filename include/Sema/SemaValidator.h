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
#include "AST/ASTParams.h"
#include "AST/ASTClassType.h"

namespace fly {

    class Sema;
    class ASTBlock;
    class ASTStmt;
    class ASTLocalVar;
    class ASTVarRef;
    class ASTNode;
    class ASTImport;
    class ASTExpr;
    class ASTParams;
    class ASTParam;
    class ASTType;
    class SourceLocation;

    class SemaValidator {

        friend class Sema;

        Sema &S;

        SemaValidator(Sema &S);

    public:

        bool CheckDuplicateParams(ASTParams *Params, ASTParam *Param);

        bool CheckDuplicateLocalVars(ASTStmt *Stmt, llvm::StringRef VarName);

        template<class T>
        bool CheckDuplicateFunctions(llvm::SmallVector<T *, 4> Functions, T *Check) {
            // Types will be checked on Resolve()
            for (T *Function : Functions) {
                for (ASTParam *Param : Function->getParams()->getList()) {
                    for (ASTParam *CheckParam: Check->getParams()->getList()) {
                        if (isEquals(Param->getType(), CheckParam->getType())) {
                            return false;
                        }
                    }
                }
            }
            return true;
        }

        bool CheckUninitialized(ASTBlock *Block, ASTVarRef *VarRef);

        bool CheckImport(ASTNode *Node, ASTImport *Import);

        bool CheckExpr(ASTExpr *Expr);

        bool isEquals(ASTType *Type1, ASTType *Type2);

        bool CheckMacroType(ASTType *Type, ASTTypeKind Kind);

        bool CheckConvertibleTypes(ASTType *FromType, ASTType *ToType);

        bool CheckSameTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2);

        bool CheckLogicalTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2);

        bool CheckClassInheritance(fly::ASTClassType *FromType, fly::ASTClassType *ToType);
    };

}  // end namespace fly

#endif // FLY_SEMA_VALIDATOR_H