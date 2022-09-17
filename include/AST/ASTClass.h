//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTClass.h - Class declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTCLASS_H
#define FLY_ASTCLASS_H

#include "ASTTopDef.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/SmallVector.h"

#include <map>

namespace fly {

    class ASTClassField;
    class ASTClassMethod;

    enum ASTClassKind {
        CLASS_STRUCT,
        CLASS_STANDARD,
        CLASS_INTERFACE,
        CLASS_ABSTRACT
    };

    enum ASTClassVisibilityKind {
        CLASS_V_DEFAULT,
        CLASS_V_PUBLIC,
        CLASS_V_PRIVATE,
        CLASS_V_PROTECTED
    };

    class ASTClassScopes {

        // Visibility of the Fields or Methods
        ASTClassVisibilityKind Visibility = CLASS_V_DEFAULT;

        // Constant of Fields or Methods
        bool Constant = false;
    };

    class ASTClass : public ASTTopDef {

        friend class SemaBuilder;
        friend class SemaResolver;

        std::string Name;

        ASTClassKind ClassKind;

        // Class Fields
        llvm::StringMap<ASTClassField *> Fields;

        // Class Methods
        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTClassMethod *, 4>>> Methods;

        ASTClass(const SourceLocation &Loc, ASTNode *Node, const std::string &Name, ASTTopScopes *Scopes);

    public:

        const std::string getName() const;

        ASTClassKind getClassKind() const;

        llvm::StringMap<ASTClassField *> getFields() const;

        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTClassMethod *, 4>>> getMethods() const;

        virtual std::string str() const;

    };
}

#endif //FLY_ASTCLASS_H
