//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFunction.h - AST Function Base header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_FUNCTIONBASE_H
#define FLY_AST_FUNCTIONBASE_H

#include "ASTBase.h"

namespace fly {

    class ASTModifier;
    class ASTVar;
    class ASTComment;
    class ASTTypeRef;
    class ASTBlockStmt;
    class SemaFunctionBase;

    class ASTFunction : public ASTBase {

        friend class ASTBuilder;
        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaResolverClass;
        friend class ParserFunction;
        friend class ParserClass;

        llvm::StringRef Name;

        // Function return type
        ASTTypeRef *ReturnTypeRef = nullptr;

        llvm::SmallVector<ASTModifier *, 8> Modifiers;

        llvm::SmallVector<ASTVar *, 8> Params;

        // Body is the main BlockStmt
        ASTBlockStmt *Body = nullptr;

        SemaFunctionBase *Sema = nullptr;

    protected:

        ASTFunction(const SourceLocation &Loc, ASTTypeRef *ReturnType,llvm::SmallVector<ASTModifier *, 8> &Modifiers,
            llvm::StringRef Name, llvm::SmallVector<ASTVar *, 8> &Params);

    public:

        llvm::StringRef getName() const;

        ASTTypeRef *getReturnTypeRef() const;

        llvm::SmallVector<ASTModifier *, 8> getModifiers() const;

        llvm::SmallVector<ASTVar *, 8> getParams() const;

        llvm::SmallVector<ASTVar *, 8> getLocalVars() const;

        ASTBlockStmt *getBody() const;

        bool isVarArg();

        SemaFunctionBase *getSema();

        std::string str() const override;
    };
}

#endif //FLY_AST_FUNCTIONBASE_H
