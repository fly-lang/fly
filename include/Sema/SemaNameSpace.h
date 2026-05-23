//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaNameSpace.h - namespace semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_NAMESPACE_H
#define FLY_SEMA_NAMESPACE_H

#include <unordered_map>

#include "Sema/SemaNode.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"

namespace fly {

    class ASTModule;
    class ASTNameSpace;
    class SemaFunction;
    class SemaClassType;
    class SemaEnumType;
    class CodeGenModule;
    class SemaType;
    class SymbolTable;

    class SemaNameSpace : public SemaNode {

        SymbolTable *Symbols;

        llvm::StringRef Name;

    public:

        explicit SemaNameSpace(llvm::StringRef Name, SymbolTable *Symbols);

    	~SemaNameSpace();

        SymbolTable *getSymbols() const;

        llvm::StringRef getName() const;

        void accept(SemaVisitor& Visitor) override;

    };
}

#endif //FLY_SEMA_NAMESPACE_H
