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

namespace fly {

    class Sema;
    class ASTBlock;
    class ASTStmt;
    class ASTLocalVar;
    class ASTVarRef;
    class ASTNode;
    class ASTImport;
    class ASTExpr;
    class ASTParam;
    class ASTType;
    class SourceLocation;

    class SemaValidator {

        friend class Sema;

        Sema &S;

        SemaValidator(Sema &S);

    public:

        bool CheckDuplicatedLocalVars(ASTStmt *Stmt, ASTLocalVar *LocalVar);

        bool CheckUndef(ASTBlock *Block, ASTVarRef *VarRef);

        bool CheckImport(ASTNode *Node, ASTImport *Import);

        bool CheckExpr(ASTExpr *Expr);

        bool isEquals(ASTParam *Param1, ASTParam *Param2);

        bool CheckMacroType(ASTType *Type, ASTMacroTypeKind Kind);

        bool CheckConvertibleTypes(ASTType *FromType, ASTType *ToType);

        bool CheckArithTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2);

        bool CheckLogicalTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2);
    };

}  // end namespace fly

#endif // FLY_SEMA_VALIDATOR_H