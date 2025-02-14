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

#include <Sym/SymVar.h>

#include "ASTBase.h"
#include "ASTTypeRef.h"

namespace fly {
    class SourceLocation;
    class CodeGenVarBase;
    class ASTVarStmt;
    class ASTScope;

    /**
     * Base Var used in:
     *  - LocalVar
     *  - GlobalVar
     *  - ClassAttribute
     */
    class ASTVar : public ASTBase
    {
        friend class ASTBuilder;
        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        SymVar* Sym = nullptr;

        ASTTypeRef* TypeRef;

        llvm::StringRef Name;

        SmallVector<ASTScope*, 8> Scopes;

        ASTExpr* Expr;

    protected:
        ASTVar(const SourceLocation& Loc, ASTTypeRef* Type, llvm::StringRef Name, SmallVector<ASTScope*, 8>& Scopes);

    public:
        SymVar* getSym() const;

        ASTTypeRef* getTypeRef() const;

        llvm::StringRef getName() const;

        bool isConstant() const;

        bool isInitialized();

        const SmallVector<ASTScope*, 8>& getScopes() const;

        ASTExpr* getExpr() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_VAR_H
