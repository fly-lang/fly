//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTUnref.h - AST Unreferenced GlobalVars and Calls
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTUNREF_H
#define FLY_ASTUNREF_H

#include "Basic/Debuggable.h"
#include <string>

namespace fly {

    class ASTNode;
    class ASTVarRef;
    class ASTFunctionCall;
    class ASTType;

    class ASTUnref : public Debuggable {

    protected:
        ASTNode *Node = nullptr;

        ASTUnref(ASTNode *Node);

    public:
        ASTNode *getNode();

        std::string str() const;
    };

    class ASTUnrefGlobalVar : public ASTUnref {

        ASTVarRef &VarRef;

        ASTUnrefGlobalVar(ASTNode *Node, ASTVarRef &VarRef);

    public:

        ASTVarRef &getVarRef();

        std::string str() const override;
    };

    class ASTUnrefFunctionCall : public ASTUnref {

        ASTFunctionCall *Call = nullptr;

        ASTUnrefFunctionCall(ASTNode *Node, ASTFunctionCall *Call);

    public:

        ASTFunctionCall *getCall();

        std::string str() const override;
    };
}

#endif //FLY_ASTUNREF_H
