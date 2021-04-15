//
// Created by marco on 4/14/21.
//

#ifndef FLY_ASTNAMESPACE_H
#define FLY_ASTNAMESPACE_H

#include "AST/GlobalVarDecl.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"

namespace fly {

    class ASTNode;

    class ASTNameSpace {

        friend ASTNode;

        llvm::StringRef NameSpace;

        // Public Global Vars
        llvm::StringMap<GlobalVarDecl *> Vars;

    public:
        ASTNameSpace(const StringRef &NS);

        const StringRef &getNameSpace() const;
    };
}

#endif //FLY_ASTNAMESPACE_H
