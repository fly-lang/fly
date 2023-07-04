//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTEnumVar.h - The Attribute in a Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTENUMVAR_H
#define FLY_ASTENUMVAR_H

#include "ASTVar.h"
#include "CodeGen/CodeGenEnumEntry.h"

namespace fly {

    class ASTEnum;

    class ASTEnumVar : public ASTVar {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTEnum *Enum = nullptr;

        const SourceLocation &Loc;

        ASTVarKind VarKind;

        llvm::StringRef Name;

        uint64_t Index;

        llvm::StringRef Comment;

        CodeGenEnumEntry *CodeGen = nullptr;

        ASTEnumVar(ASTEnum *Enum, const SourceLocation &Loc, llvm::StringRef Name, uint64_t Index);

    public:

        ASTEnum *getEnum() const;

        const SourceLocation &getLocation() const;

        ASTVarKind getVarKind() override;

        llvm::StringRef getName() const override;

        uint64_t getIndex() const;

        ASTType *getType() const override;

        ASTExpr *getExpr() const override;

        llvm::StringRef getComment() const;

        CodeGenEnumEntry *getCodeGen() const;

        void setCodeGen(CodeGenEnumEntry *CGE);

        std::string print() const;

        std::string str() const;
    };
}

#endif //FLY_ASTCLASSVAR_H
