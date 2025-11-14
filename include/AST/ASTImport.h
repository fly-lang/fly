//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTImport.h - AST Import header
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

    class ASTIdentifier;
    class ASTModule;

    class ASTImport : public ASTNode {

        friend class ASTBuilder;

        ASTModule *Module;

        ASTIdentifier *Identifier;

        ASTIdentifier *Alias;

        ASTImport(ASTIdentifier *Identifier, ASTIdentifier *Alias = nullptr);

    public:

        ~ASTImport() override;

        void accept(ASTVisitor& Visitor) override;

        ASTModule* getModule() const;

        ASTIdentifier* getImport() const;

        ASTIdentifier *getAlias() const;

        std::string str() const override;
    };

}

#endif //FLY_AST_IMPORT_H
