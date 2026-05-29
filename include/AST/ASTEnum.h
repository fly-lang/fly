//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTEnum.h - AST enum type definition header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_ENUM_H
#define FLY_AST_ENUM_H

#include "ASTNode.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

namespace fly {

    class ASTModifier;
    class ASTType;

    class ASTEnum : public ASTNode {

        friend class ASTBuilder;

        llvm::SmallVector<ASTNode *, 8> Nodes;

        llvm::SmallVector<ASTModifier *, 8> Modifiers;

        llvm::StringRef Name;

        llvm::SmallVector<ASTType *, 4> Bases;

        ASTEnum(const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                 llvm::SmallVector<ASTType *, 4> &Bases);

    public:

        ~ASTEnum() override;

        void accept(ASTVisitor& Visitor) override;

        llvm::SmallVector<ASTNode*, 8> getNodes() const;

        llvm::SmallVector<ASTModifier*, 8> getModifiers() const;

        llvm::StringRef getName() const;

        llvm::SmallVector<ASTType*, 4> getBases() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_ENUM_H
