//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFunction.h - AST Function header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_FUNCTION_H
#define FLY_AST_FUNCTION_H

#include "ASTTopDef.h"
#include "ASTFunctionBase.h"
#include "CodeGen/CodeGenFunction.h"

#include <vector>

namespace fly {

    class ASTNode;
    class ASTParams;
    class ASTType;
    class ASTBlock;
    class ASTScopes;
    class CodeGenFunction;

    /**
     * The Function Declaration and definition
     * Ex.
     *   int func() {
     *     return 1
     *   }
     */
    class ASTFunction : public ASTFunctionBase, public virtual ASTTopDef {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class FunctionParser;

        ASTTopDefKind TopDefKind = ASTTopDefKind::DEF_FUNCTION;

        ASTNode *Node;

        // Populated during codegen phase
        CodeGenFunction *CodeGen = nullptr;

        ASTFunction(const SourceLocation &Loc, ASTNode *Node, ASTType *ReturnType, llvm::StringRef Name,
                    ASTScopes *Scopes);

    public:

        ASTTopDefKind getTopDefKind() const override;

        ASTNode *getNode() const override;

        ASTNameSpace *getNameSpace() const override;

        llvm::StringRef getName() const override;

        CodeGenFunction *getCodeGen() const override;

        void setCodeGen(CodeGenFunction *CGF);

        std::string str() const override;
    };
}

#endif //FLY_AST_FUNCTION_H
