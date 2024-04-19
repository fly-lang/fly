//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTEnumVar.h - The Attribute in a Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTENUMENTRY_H
#define FLY_ASTENUMENTRY_H

#include "AST/ASTVar.h"
#include "CodeGen/CodeGenEnumEntry.h"

namespace fly {

    class ASTEnum;

    class ASTEnumEntry : public ASTVar {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTEnum *Enum;

        uint32_t Index = 0;

        llvm::StringRef Comment;

        CodeGenEnumEntry *CodeGen = nullptr;

        ASTEnumEntry(ASTEnum *Enum, const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ASTEnum *getEnum() const;

        uint32_t getIndex() const;

        CodeGenEnumEntry *getCodeGen() const;

        void setCodeGen(CodeGenEnumEntry *CGE);

        std::string print() const;

        std::string str() const;
    };
}

#endif //FLY_ASTCLASSVAR_H
