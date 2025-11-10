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

#include "AST/ASTNode.h"
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class InputFile;
    class ASTNameSpace;
    class FileID; // TODO implement in Id

    class ASTModule : public ASTNode {

        friend class ASTBuilder;

        // Source File
        InputFile *File;

        // Namespace declaration
        ASTNameSpace* NameSpace = nullptr;

        // Module FileName
        const std::string Name;

        // Module is Header
        const bool Header;

        // All Top Definitions sorted in the order it appears in the code: NameSpace, Imports, Global Vars, Functions
        llvm::SmallVector<ASTNode *, 8> Nodes;

        ASTModule() = delete;

        ASTModule(InputFile *F);

    public:

        ~ASTModule() override;

        void accept(ASTVisitor& Visitor) override;

        InputFile *getFile() const;

        bool isHeader() const;

        std::string getName();

        ASTNameSpace* getNameSpace();

        const llvm::SmallVector<ASTNode *, 8> &getNodes() const;

        std::string str() const;
    };
}

#endif //FLY_AST_MODULE_H
