//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTEnum.h - AST Enum header
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

    class ASTModule;
    class ASTModifier;
    class ASTTypeRef;

    class ASTEnum : public ASTNode {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        ASTModule *Module;

        llvm::SmallVector<ASTNode *, 8> Definitions;

        llvm::SmallVector<ASTModifier *, 8> Modifiers;

        llvm::StringRef Name;

        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses; // FIXME ?

        ASTEnum(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                 llvm::SmallVector<ASTTypeRef *, 4> &SuperClasses);

    public:

        ASTModule* getModule() const;

        llvm::SmallVector<ASTNode*, 8> getDefinitions() const;

        llvm::SmallVector<ASTModifier*, 8> getModifiers() const;

        llvm::StringRef getName() const;

        llvm::SmallVector<ASTTypeRef*, 4> getSuperClasses() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_ENUM_H
