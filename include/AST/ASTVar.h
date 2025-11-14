//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.h - AST Var header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_VAR_H
#define FLY_AST_VAR_H

#include <Sema/SemaVar.h>

#include "ASTNode.h"

namespace fly {

    class SourceLocation;
    class CodeGenVarBase;
    class ASTVarStmt;
    class ASTModifier;
    class SemaVar;
    class ASTType;
	class ASTExpr;

    /**
     * Base Var used in:
     *  - LocalVar
     *  - GlobalVar
     *  - ClassAttribute
     */
    class ASTVar : public ASTNode {

        friend class ASTBuilder;

        SemaVar* Sema = nullptr;

        ASTType* TypeRef;

        llvm::StringRef Name;

        SmallVector<ASTModifier*, 8> Modifiers;

        ASTExpr* Expr;

    protected:
        ASTVar(const SourceLocation& Loc, ASTType* Type, llvm::StringRef Name, SmallVector<ASTModifier*, 8>& Modifiers);

    public:

        void accept(ASTVisitor& Visitor) override;

        SemaVar* getSema() const;

        ASTType* getTypeRef() const;

        llvm::StringRef getName() const;

        const SmallVector<ASTModifier*, 8>& getModifiers() const;

        ASTExpr* getExpr() const;

        void setExpr(ASTExpr* Expr);

        std::string str() const override;
    };
}

#endif //FLY_AST_VAR_H
