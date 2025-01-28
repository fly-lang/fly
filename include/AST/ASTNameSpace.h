//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNameSpace.h - AST Namespace header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_NAMESPACE_H
#define FLY_AST_NAMESPACE_H

#include "ASTIdentifier.h"

namespace fly {

    class ASTModule;
    class ASTContext;
    class ASTGlobalVar;
    class ASTIdentity;
    class ASTFunction;
    class ASTImport;
    class ASTComment;
    class ASTIdentityType;

    class ASTNameSpace : public ASTIdentifier {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTModule *Module;

        ASTNameSpace(const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ~ASTNameSpace();

        ASTModule* getModule() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_NAMESPACE_H
