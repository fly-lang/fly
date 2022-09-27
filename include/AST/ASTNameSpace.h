//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNameSpace.h - AST Namespace
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTNAMESPACE_H
#define FLY_ASTNAMESPACE_H

#include "AST/ASTNodeBase.h"

namespace fly {

    class ASTNode;
    class ASTContext;
    class ASTGlobalVar;
    class ASTClass;
    class ASTUnrefGlobalVar;
    class ASTUnrefFunctionCall;

    class ASTNameSpace : public ASTNodeBase {

        friend class Sema;
        friend class SemaResolver;
        friend class SemaBuilder;

        // AST by FileID
        llvm::StringMap<ASTNode *> Nodes;

        bool ExternalLib;

        // Classes
        llvm::StringMap<ASTClass *> Classes;

        ASTNameSpace(std::string NameSpace, ASTContext *Context, bool ExternalLib = false);

        ~ASTNameSpace();

    public:

        static const std::string DEFAULT;

        const llvm::StringMap<ASTNode *> &getNodes() const;

        bool isExternalLib() const;

        const llvm::StringMap<ASTClass *> &getClasses() const;

        virtual std::string str() const;
    };
}

#endif //FLY_ASTNAMESPACE_H
