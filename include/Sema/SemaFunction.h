//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaFunction.h - Symbolic Table of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_FUNCTION_H
#define FLY_SEMA_FUNCTION_H

#include "SemaFunctionBase.h"
#include "SemaVisibilityKind.h"
#include "CodeGen/CodeGenFunction.h"

namespace fly {

    class ASTFunction;
    class CodeGenFunction;
    class SemaComment;
    class SemaModule;
    enum class SemaVisibilityKind;

    class SemaFunction : public SemaFunctionBase {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

        SemaModule *Module;

        SemaComment *Comment = nullptr;

        SemaVisibilityKind Visibility = SemaVisibilityKind::DEFAULT;

        // Populated during codegen phase
        CodeGenFunction *CodeGen = nullptr;

        explicit SemaFunction(ASTFunction &AST);

        std::string MangleFunction(ASTFunction *AST);

    public:

        SemaModule *getModule() const;

        SemaComment *getComment() const;

        SemaVisibilityKind getVisibility() const;

        CodeGenFunction *getCodeGen() const override;

        void setCodeGen(CodeGenFunction *CGF);
    };

}  // end namespace fly

#endif // FLY_SEMA_FUNCTION_H