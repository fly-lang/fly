//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTEnum.h - Class declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTENUM_H
#define FLY_ASTENUM_H

#include "ASTTopDef.h"
#include "Basic/Debuggable.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/SmallVector.h"

#include <map>

namespace fly {

    class CodeGenEnum;
    class ASTEnumType;
    class ASTEnumVar;

    class ASTEnum : public ASTTopDef {

        friend class SemaBuilder;
        friend class SemaResolver;

        llvm::StringRef Name;

        // Source Location
        SourceLocation Location;

        ASTEnumType *Type = nullptr;

        llvm::SmallVector<ASTEnumType *, 4> SuperClasses;

        // Class Fields
        llvm::StringMap<ASTEnumVar *> Vars;

        CodeGenEnum *CodeGen = nullptr;

        ASTEnum(ASTNode *Node, ASTScopes *Scopes,
                 const SourceLocation &Loc, llvm::StringRef Name,
                 llvm::SmallVector<ASTEnumType *, 4> &ExtClasses);

    public:

        llvm::StringRef getName() const;

        const SourceLocation &getLocation() const;

        ASTEnumType *getType() const;

        llvm::StringMap<ASTEnumVar *> getVars() const;

        CodeGenEnum *getCodeGen() const;

        void setCodeGen(CodeGenEnum *CGC);

        std::string print() const;

        std::string str() const;

    };
}

#endif //FLY_ASTENUM_H
