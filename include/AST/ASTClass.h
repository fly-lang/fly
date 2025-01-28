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

#include "ASTIdentity.h"
#include "llvm/ADT/StringMap.h"

namespace fly {

    class ASTModule;
    class ASTClassAttribute;
    class ASTClassMethod;
    class CodeGenClass;
    class ASTBlockStmt;
    class ASTAssignmentStmt;
    class ASTClassType;

    enum class ASTClassKind {
        STRUCT, // has only Fields
        CLASS, // has only Fields and Methods defined
        INTERFACE, // has only Methods declarations
    };

    class ASTClass : public ASTIdentity {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTClassKind ClassKind;

        ASTVisibilityKind Visibility;

        llvm::SmallVector<ASTClassType *, 4> SuperClasses;

        // Class Fields
        llvm::SmallVector<ASTClassAttribute *, 8> Attributes;

        ASTClassMethod *DefaultConstructor = nullptr;

        // Class Constructors
        llvm::SmallVector <ASTClassMethod *, 8> Constructors;

        // Class Methods
        llvm::SmallVector <ASTClassMethod *, 8> Methods;

        CodeGenClass *CodeGen = nullptr;

        ASTClass(ASTModule *Module, ASTClassKind ClassKind, llvm::SmallVector<ASTScope *, 8> &Scopes,
                 const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTClassType *, 4> &ClassTypes);

    public:

        ASTModule* getModule() const;

        ASTClassKind getClassKind() const;

        llvm::SmallVector<ASTClassType *, 4> getSuperClasses() const;

        llvm::SmallVector<ASTClassAttribute *, 8> getAttributes() const;

        ASTClassMethod *getDefaultConstructor() const;

        llvm::SmallVector<ASTClassMethod *, 8> getConstructors() const;

        llvm::SmallVector<ASTClassMethod *, 8> getMethods() const;

        CodeGenClass *getCodeGen() const;

        void setCodeGen(CodeGenClass *CGC);

        std::string str() const override;

    };
}

#endif //FLY_AST_CLASS_H
