//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTClass.h - AST Class header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_CLASS_H
#define FLY_AST_CLASS_H

#include "ASTNode.h"

namespace fly {

    class ASTModifier;
    class ASTClassAttribute;
    class CodeGenClass;
    class ASTBlockStmt;
    class ASTAssignStmt;
    class ASTType;

    enum class ASTClassKind {
        CLASS,
        INTERFACE,
        STRUCT
    };

    class ASTClass : public ASTNode {

        friend class ASTBuilder;

        llvm::SmallVector<ASTNode *, 8> Nodes;

        llvm::SmallVector<ASTModifier *, 8> Modifiers;

        llvm::StringRef Name;

        ASTClassKind ClassKind;

        llvm::SmallVector<ASTType *, 4> Bases;

        ASTClass(ASTClassKind ClassKind, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                 const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTType *, 4> &Bases);

    public:

        ~ASTClass() override;

        void accept(ASTVisitor& Visitor) override;

        llvm::SmallVector<ASTNode *, 8> getNodes() const;

        ASTClassKind getClassKind() const;

        const llvm::SmallVector<ASTType *, 4> &getBases() const;

        llvm::SmallVector<ASTModifier *, 8> getModifiers() const;

        llvm::StringRef getName() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_CLASS_H
