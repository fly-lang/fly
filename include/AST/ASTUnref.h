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

#include <string>

namespace fly {

    class ASTNode;
    class ASTVarRef;
    class ASTFuncCall;
    class ASTType;

    class ASTUnref {

    protected:
        ASTNode *Node;

        ASTUnref(ASTNode *Node);

    public:
        ASTNode *getNode();

        virtual std::string str() const = 0;
    };

    class ASTUnrefGlobalVar : public ASTUnref {

        ASTVarRef &VarRef;

    public:
        ASTUnrefGlobalVar(ASTNode *Node, ASTVarRef &VarRef);

        ASTVarRef &getVarRef();

        std::string str() const;
    };

    class ASTUnrefCall : public ASTUnref {

        ASTFuncCall *Call;

    public:
        ASTUnrefCall(ASTNode *Node, ASTFuncCall *Call);

        ASTFuncCall *getCall();

        std::string str() const;
    };
}

#endif //FLY_ASTUNREF_H
