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

    class ASTModule;
    class ASTModifier;
    class ASTClassAttribute;
    class CodeGenClass;
    class ASTBlockStmt;
    class ASTVarStmt;
    class ASTTypeRef;

    enum class ASTClassKind {
        CLASS,
        INTERFACE,
        STRUCT
    };

    class ASTClass : public ASTNode {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        ASTModule *Module;

        llvm::SmallVector<ASTNode *, 8> Definitions;

        llvm::SmallVector<ASTModifier *, 8> Modifiers;

        llvm::StringRef Name;

        ASTClassKind ClassKind;

        llvm::SmallVector<ASTTypeRef *, 4> BaseClasses;

        ASTClass(ASTModule *Module, ASTClassKind ClassKind, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                 const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTTypeRef *, 4> &SuperClasses);

    public:

        ASTModule* getModule() const;

        llvm::SmallVector<ASTNode *, 8> getDefinitions() const;

        ASTClassKind getClassKind() const;

        llvm::SmallVector<ASTTypeRef *, 4> getBaseClasses() const;

        llvm::SmallVector<ASTModifier *, 8> getModifiers() const;

        llvm::StringRef getName() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_CLASS_H
