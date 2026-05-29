//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFunction.h - AST function definition header
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
    class ASTTypeParam;

    enum class ASTFunctionKind {
        F_FUNCTION,
        F_METHOD,
    };

    class ASTFunction : public ASTNode {

        friend class ASTBuilder;
        friend class ParserClass;
        friend class Parser;

        ASTFunctionKind FunctionKind;

        llvm::StringRef Name;

        llvm::SmallVector<ASTModifier *, 8> Modifiers;

        llvm::SmallVector<ASTParam *, 8> Params;

        // Explicit return type for single-return functions (null = void)
        ASTType *ReturnType = nullptr;

        // Multiple return types (set only when > 1 return type is declared)
        llvm::SmallVector<ASTType *, 4> ReturnTypes;

        // Body is the main BlockStmt
        ASTBlockStmt *Body = nullptr;

        // Type parameters for generic functions/methods: void foo<T>(T v)
        llvm::SmallVector<ASTTypeParam *, 4> TypeParams;

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

        const llvm::SmallVector<ASTTypeParam *, 4> &getTypeParams() const;

        ASTType *getReturnType() const;

        void setReturnType(ASTType *RT);

        const llvm::SmallVector<ASTType *, 4> &getReturnTypes() const;

        void setReturnTypes(const llvm::SmallVector<ASTType *, 4> &RTs);

        ASTBlockStmt *getBody() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_FUNCTION_H
