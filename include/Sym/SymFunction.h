//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SymFunction.h - Symbolic Table of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_FUNCTION_H
#define FLY_SYM_FUNCTION_H

#include "SymFunctionBase.h"
#include "SymVisibilityKind.h"
#include "CodeGen/CodeGenFunction.h"

namespace fly {

    class ASTFunction;
    class CodeGenFunction;
    class SymComment;
    class SymModule;
    enum class SymVisibilityKind;

    class SymFunction : public SymFunctionBase {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        SymModule *Module;

        const std::string MangledName;

        SymComment *Comment = nullptr;

        SymVisibilityKind Visibility = SymVisibilityKind::DEFAULT;

        // Populated during codegen phase
        CodeGenFunction *CodeGen = nullptr;

        explicit SymFunction(ASTFunction *AST);

    public:

        SymModule *getModule() const;

        std::string getMangledName() const;

        SymComment *getComment() const;

        SymVisibilityKind getVisibility() const;

        CodeGenFunction *getCodeGen() const override;

        void setCodeGen(CodeGenFunction *CGF);
    };

}  // end namespace fly

#endif // FLY_SYM_FUNCTION_H