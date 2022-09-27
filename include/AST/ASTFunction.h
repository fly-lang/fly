//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFunction.h - Function declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_FUNCTION_H
#define FLY_FUNCTION_H

#include "ASTTopDef.h"
#include "ASTFunctionBase.h"

#include <vector>

namespace fly {

    class ASTNode;
    class ASTParams;
    class ASTType;
    class ASTBlock;

    /**
     * The Function Declaration and definition
     * Ex.
     *   int func() {
     *     return 1
     *   }
     */
    class ASTFunction : public ASTFunctionBase, public ASTTopDef {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class FunctionParser;

        ASTFunction(const SourceLocation &Loc, ASTNode *Node, ASTType *ReturnType, const std::string Name,
                    ASTTopScopes *Scopes);

    public:

        const std::string getName() const override;

        std::string str() const;
    };
}

#endif //FLY_FUNCTION_H
