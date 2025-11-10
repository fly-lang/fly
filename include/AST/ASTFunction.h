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

#include "ASTNode.h"

namespace fly {

    class ASTModifier;
    class ASTVar;
    class ASTComment;
    class ASTType;
    class ASTBlockStmt;

    class ASTFunction : public ASTNode {

        friend class ASTBuilder;

        llvm::StringRef Name;

        // Function return type
        ASTType *ReturnTypeRef = nullptr;

        llvm::SmallVector<ASTModifier *, 8> Modifiers;

        llvm::SmallVector<ASTVar *, 8> Params;

        // Body is the main BlockStmt
        ASTBlockStmt *Body = nullptr;

    protected:

        ASTFunction(const SourceLocation &Loc, ASTType *ReturnType,llvm::SmallVector<ASTModifier *, 8> &Modifiers,
            llvm::StringRef Name, llvm::SmallVector<ASTVar *, 8> &Params);

    public:

        void accept(ASTVisitor& Visitor) override;

        llvm::StringRef getName() const;

        ASTType *getReturnTypeRef() const;

        llvm::SmallVector<ASTModifier *, 8> getModifiers() const;

        llvm::SmallVector<ASTVar *, 8> getParams() const;

        llvm::SmallVector<ASTVar *, 8> getLocalVars() const;

        ASTBlockStmt *getBody() const;

        bool isVarArg();

        std::string str() const override;
    };
}

#endif //FLY_AST_FUNCTIONBASE_H
