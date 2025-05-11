//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaGlobalVar.h - Symbolic Table of GlobalVar
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_GLOBALVAR_H
#define FLY_SEMA_GLOBALVAR_H

#include "SemaVisibilityKind.h"
#include "Sema/SemaVar.h"
#include "CodeGen/CodeGenGlobalVar.h"

namespace fly {

    class ASTVar;
    class SemaComment;
    class CodeGenGlobalVar;
    class SemaModule;
    enum class SemaVisibilityKind;

    class SemaGlobalVar  : public SemaVar {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        SemaModule *Module;

        SemaComment *Comment = nullptr;

        SemaVisibilityKind Visibility = SemaVisibilityKind::DEFAULT;

        // Code Generator
        CodeGenGlobalVar *CodeGen = nullptr;

        explicit SemaGlobalVar(ASTVar *GlobalVar);

    public:

        SemaModule *getModule() const;

        SemaComment *getComment() const;

        SemaVisibilityKind getVisibility() const;

        CodeGenGlobalVar *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase *CGV) override;

    };

}  // end namespace fly

#endif // FLY_SEMA_GLOBALVAR_H