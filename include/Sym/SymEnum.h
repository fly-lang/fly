//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassSymbols.h - SemaClassSymbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_ENUM_H
#define FLY_SYM_ENUM_H

#include <AST/ASTScopes.h>

#include "SymVisibilityKind.h"
#include "Sym/SymType.h"
#include "llvm/ADT/StringMap.h"

namespace fly {

    class Sema;
    class ASTEnum;
    class SymComment;
    class SymEnumEntry;
    class SymModule;
    enum class SymVisibilityKind;

    class SymEnum : public SymType {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTEnum *AST;

        SymModule *Module;

        // Super Enums
        llvm::StringMap<SymEnum *> SuperEnums;

        SymVisibilityKind Visibility = SymVisibilityKind::DEFAULT;

        bool Constant;

        // Enum Entries
        llvm::StringMap<SymEnumEntry *> Entries;

        SymComment *Comment = nullptr;

        explicit SymEnum(ASTEnum *AST);

    public:

        ASTEnum *getAST();

        SymModule *getModule() const;

        const llvm::StringMap<SymEnum *> &getSuperEnums() const;

        SymVisibilityKind getVisibility() const;

        bool isConstant() const;

        const llvm::StringMap<SymEnumEntry *> &getEntries() const;

        SymComment *getComment() const;

    };

}  // end namespace fly

#endif // FLY_SYM_IDENTITY_H