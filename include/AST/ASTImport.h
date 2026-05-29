//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTImport.h - AST import declaration header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_IMPORT_H
#define FLY_AST_IMPORT_H

#include "ASTNode.h"

namespace fly {

    class ASTName;
    class ASTModule;

    class ASTImport : public ASTNode {

        friend class ASTBuilder;

        llvm::SmallVector<ASTName *, 4> Names;

        llvm::SmallVector<ASTName *, 4> Alias;

        bool Wildcard = false;

        ASTImport(const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> &Names,
                  llvm::SmallVector<ASTName *, 4> &Alias, bool Wildcard);

    public:

        ~ASTImport() override;

        void accept(ASTVisitor& Visitor) override;

        const llvm::SmallVector<ASTName *, 4> &getNames() const;

        const llvm::SmallVector<ASTName *, 4> &getAlias() const;

        bool isWildcard() const;

        std::string str() const override;
    };

}

#endif //FLY_AST_IMPORT_H
