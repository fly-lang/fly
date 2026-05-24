//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaImport.h - import semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_IMPORT_H
#define FLY_SEMA_IMPORT_H

#include <llvm/ADT/SmallVector.h>

#include "SemaNode.h"

namespace fly {

	class ASTImport;
    class ASTName;
    class SymbolTable;

    /**
     * AST Context
     */
    class SemaImport : public SemaNode {

        ASTImport &AST;

        SymbolTable *Symbols = nullptr;

    public:
        SemaImport(ASTImport &AST);

        ~SemaImport() override = default;

		ASTImport* getAST() const;

        const llvm::SmallVector<ASTName *, 4> &getNames() const;

        const llvm::SmallVector<ASTName *, 4> &getTarget() const;

        bool isWildcard() const;

        SymbolTable *getSymbols() const;

        void setSymbols(SymbolTable *Symbols);

        void addSymbol(SymbolTable *Symbols);

        std::string str() const override;

        void accept(SemaVisitor& Visitor) override;
    };
}

#endif //FLY_SEMA_IMPORT_H
