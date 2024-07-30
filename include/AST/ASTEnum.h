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

#include "ASTIdentity.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"

#include <map>

namespace fly {

    class ASTEnumEntry;
    class ASTEnumType;

    class ASTEnum : public ASTIdentity {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTEnumType *Type = nullptr;

        llvm::SmallVector<ASTEnumType *, 4> SuperClasses; // FIXME ?

        // Class Fields
        llvm::SmallVector<ASTEnumEntry *, 8> Entries;

        ASTEnum(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                 llvm::SmallVector<ASTEnumType *, 4> &ExtClasses);

    public:

        llvm::SmallVector<ASTEnumEntry *, 8> getEntries() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_ENUM_H
