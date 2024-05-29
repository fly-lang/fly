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
#include "ASTClassType.h"

#include "llvm/ADT/StringMap.h"

#include <map>

namespace fly {

    class ASTClassAttribute;
    class ASTClassMethod;
    class CodeGenClass;
    class ASTBlockStmt;

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

        llvm::SmallVector<ASTClassType *, 4> SuperClasses;

        // Class Fields
        llvm::StringMap<ASTClassAttribute *> Attributes;

        bool autoDefaultConstructor = false;

        // Class Constructors
        std::map <uint64_t, llvm::SmallVector <ASTClassMethod *, 4>> Constructors;

        // Class Methods
        llvm::StringMap<std::map <uint64_t, llvm::SmallVector <ASTClassMethod *, 4>>> Methods;

        CodeGenClass *CodeGen = nullptr;

        ASTClass(ASTClassKind ClassKind, ASTScopes *Scopes, const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ASTClassKind getClassKind() const;

        llvm::SmallVector<ASTClassType *, 4> getSuperClasses() const;

        llvm::StringMap<ASTClassAttribute *> getAttributes() const;

        std::map <uint64_t,llvm::SmallVector <ASTClassMethod *, 4>> getConstructors() const;

        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTClassMethod *, 4>>> getMethods() const;

        CodeGenClass *getCodeGen() const;

        void setCodeGen(CodeGenClass *CGC);

        std::string str() const override;

    };
}

#endif //FLY_AST_CLASS_H
