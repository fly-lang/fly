//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFunction.h - AST Function Base header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_FUNCTION_H
#define FLY_AST_FUNCTION_H

#include "ASTNode.h"

namespace fly {

    class ASTModifier;
    class ASTParam;
    class ASTComment;
    class ASTBlockStmt;
    class ASTType;

    enum class ASTFunctionKind {
        F_FUNCTION,
        F_METHOD,
    };

    class ASTFunction : public ASTNode {

        friend class ASTBuilder;

        ASTFunctionKind FunctionKind;

        llvm::StringRef Name;

        llvm::SmallVector<ASTModifier *, 8> Modifiers;

        llvm::SmallVector<ASTParam *, 8> Params;

        // Explicit return type (only set from .fly.h header declarations; null = void)
        ASTType *ReturnType = nullptr;

        // Body is the main BlockStmt
        ASTBlockStmt *Body = nullptr;

    protected:

        ASTFunction(const SourceLocation &Loc, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
            llvm::StringRef Name, llvm::SmallVector<ASTParam *, 8> &Params,
            ASTFunctionKind FunctionKind = ASTFunctionKind::F_FUNCTION);

    public:

        ~ASTFunction() override;

        void accept(ASTVisitor& Visitor) override;

        llvm::StringRef getName() const;


        llvm::SmallVector<ASTModifier *, 8> getModifiers() const;

        llvm::SmallVector<ASTParam *, 8> getParams() const;

        ASTType *getReturnType() const;

        void setReturnType(ASTType *RT);

        ASTBlockStmt *getBody() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_FUNCTION_H
