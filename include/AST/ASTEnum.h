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
#include "ASTEnumType.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"

#include <map>

namespace fly {

    class CodeGenEnum;
    class ASTEnumEntry;

    class ASTEnum : public ASTIdentity {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTEnumType *Type = nullptr;

        llvm::SmallVector<ASTEnumType *, 4> SuperClasses; // FIXME ?

        // Class Fields
        llvm::StringMap<ASTEnumEntry *> Vars;

        CodeGenEnum *CodeGen = nullptr;

        ASTEnum(ASTNode *Node, ASTScopes *Scopes,
                 const SourceLocation &Loc, llvm::StringRef Name,
                 llvm::SmallVector<ASTEnumType *, 4> &ExtClasses);

    public:

        ASTEnumType *getType() override;

        llvm::StringMap<ASTEnumEntry *> getVars() const;

        CodeGenEnum *getCodeGen() const;

        void setCodeGen(CodeGenEnum *CGC);

        std::string print() const override;

        std::string str() const override;

    };
}

#endif //FLY_AST_ENUM_H
