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
#include "SemaLocalVar.h"
#include "SemaVisibilityKind.h"
#include "CodeGen/CodeGenFunction.h"

namespace fly {

    class ASTFunction;
    class CodeGenFunction;
    class SemaComment;
    class SemaModule;
    class SymbolTable;
    enum class SemaVisibilityKind;

    class SemaFunction : public SemaFunctionBase {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

        SemaModule *Module;

        SymbolTable *Symbols;

        SemaComment *Comment = nullptr;

        SemaVisibilityKind Visibility;

        llvm::SmallVector<SemaLocalVar *, 4> LocalVars;

        // Populated during codegen phase
        CodeGenFunction *CodeGen = nullptr;

        explicit SemaFunction(ASTFunction &AST, SymbolTable *Symbols);

    public:

        SemaModule *getModule() const;

        SymbolTable *getSymbols() const;

        SemaComment *getComment() const;

        SemaVisibilityKind getVisibility() const;

        const llvm::SmallVector<SemaLocalVar *, 4> &getLocalVars() const;

        CodeGenFunction *getCodeGen() const override;

        void setCodeGen(CodeGenFunction *CGF);
    };

}  // end namespace fly

#endif // FLY_SEMA_FUNCTION_H