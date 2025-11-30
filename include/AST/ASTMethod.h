//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTMethod.h - AST Function Base header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_METHOD_H
#define FLY_AST_METHOD_H

#include "ASTFunction.h"

namespace fly {

    class ASTModifier;
    class ASTParam;
    class ASTComment;
    class ASTType;
    class ASTBlockStmt;

    class ASTMethod : public ASTFunction {

        friend class ASTBuilder;

    protected:

        ASTMethod(const SourceLocation &Loc, ASTType *ReturnType,llvm::SmallVector<ASTModifier *, 8> &Modifiers,
            llvm::StringRef Name, llvm::SmallVector<ASTParam *, 8> &Params);

    public:

        void accept(ASTVisitor& Visitor) override;

    };
}

#endif //FLY_AST_FUNCTION_H
