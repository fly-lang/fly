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

#include "ASTBase.h"

namespace fly {

    class ASTModule;
    class ASTScope;
    class ASTClassAttribute;
    class CodeGenClass;
    class ASTBlockStmt;
    class ASTAssignmentStmt;
class ASTTypeRef;

    enum class ASTClassKind {
        CLASS,
        INTERFACE,
        STRUCT
    };

    class ASTClass : public ASTBase {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTModule *Module;

        llvm::SmallVector<ASTBase *, 8> Definitions;

        llvm::SmallVector<ASTScope *, 8> Scopes;

        llvm::StringRef Name;

        ASTClassKind ClassKind;

        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;

        ASTClass(ASTModule *Module, ASTClassKind ClassKind, llvm::SmallVector<ASTScope *, 8> &Scopes,
                 const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTTypeRef *, 4> &SuperClasses);

    public:

        ASTModule* getModule() const;

        llvm::SmallVector<ASTBase *, 8> getDefinitions() const;

        ASTClassKind getClassKind() const;

        llvm::SmallVector<ASTTypeRef *, 4> getSuperClasses() const;

        llvm::SmallVector<ASTScope *, 8> getScopes() const;

        llvm::StringRef getName() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_CLASS_H
