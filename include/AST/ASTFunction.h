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

#include "ASTFunctionBase.h"
#include "CodeGen/CodeGenFunction.h"

namespace fly {

    class ASTModule;
    class ASTNameSpace;
    enum class ASTVisibilityKind;

    /**
     * The Function Declaration and definition
     * Ex.
     *   int func() {
     *     return 1
     *   }
     */
    class ASTFunction : public ASTFunctionBase {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTVisibilityKind Visibility;

        ASTModule *Module;

        // Function Name
        llvm::StringRef Name;

        // Populated during codegen phase
        CodeGenFunction *CodeGen = nullptr;

        ASTFunction(ASTModule *Module, const SourceLocation &Loc, ASTType *ReturnType, llvm::StringRef Name,
                    llvm::SmallVector<ASTScope *, 8> &Scopes, llvm::SmallVector<ASTParam *, 8> &Params);

    public:

        llvm::StringRef getName() const;

        ASTVisibilityKind getVisibility() const;

        ASTModule *getModule() const;

        ASTNameSpace *getNameSpace() const;

        CodeGenFunction *getCodeGen() const override;

        void setCodeGen(CodeGenFunction *CGF);

        std::string str() const override;
    };
}

#endif //FLY_AST_FUNCTION_H
