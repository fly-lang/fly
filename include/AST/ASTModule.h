//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTModule.h - AST Module header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_MODULE_H
#define FLY_AST_MODULE_H

#include "AST/ASTBase.h"
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class ASTNameSpace;
    class FileID; // TODO implement in Id

    class ASTModule {

        friend class Sema;
        friend class SemaResolver;
        friend class ASTBuilder;

        // Namespace declaration
        ASTNameSpace* NameSpace = nullptr;

        // Module Id
        const size_t Id;

        // Module FileName
        const std::string Name;

        // Module is Header
        const bool Header;

        // All Top Definitions sorted in the order it appears in the code: NameSpace, Imports, Global Vars, Functions
        llvm::SmallVector<ASTBase *, 8> Definitions;

        ASTModule() = delete;

        ~ASTModule();

        ASTModule(size_t& Id, std::string Name, bool isHeader);

    public:
        const size_t getId() const;

        bool isHeader() const;

        std::string getName();

        ASTNameSpace* getNameSpace();

        const llvm::SmallVector<ASTBase *, 8> &getDefinitions() const;

        std::string str() const;
    };
}

#endif //FLY_AST_MODULE_H
