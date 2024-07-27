//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTEnumEntry.h - AST Enum Entry header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_ENUMENTRY_H
#define FLY_AST_ENUMENTRY_H

#include "AST/ASTVar.h"

namespace fly {

    class ASTEnum;
    class ASTEnumType;
    class CodeGenEnumEntry;

    class ASTEnumEntry : public ASTVar {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        llvm::StringRef Name;

        uint32_t Index;

        ASTEnum *Enum = nullptr;

        llvm::StringRef Comment;

        CodeGenEnumEntry *CodeGen = nullptr;

        ASTEnumEntry(const SourceLocation &Loc, ASTEnumType *Type, llvm::StringRef Name,
                     SmallVector<ASTScope *, 8> &Scopes);

    public:

        ASTEnum *getEnum() const;

        uint32_t getIndex() const;

        CodeGenVarBase *getCodeGen() const override;

        void setCodeGen(CodeGenEnumEntry *CG);

        std::string str() const override;
    };
}

#endif //FLY_AST_ENUMENTRY_H
