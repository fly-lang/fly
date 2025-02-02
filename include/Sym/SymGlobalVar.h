//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SymGlobalVar.h - Symbolic Table of GlobalVar
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_GLOBALVAR_H
#define FLY_SYM_GLOBALVAR_H

#include "Sym/SymVar.h"
#include "CodeGen/CodeGenGlobalVar.h"

namespace fly {

    class ASTVar;
    class SymComment;
    class CodeGenGlobalVar;
    class SymModule;
    enum class SymVisibilityKind;

    class SymGlobalVar  : public SymVar {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        SymModule *Module;

        SymComment *Comment = nullptr;

        // Code Generator
        CodeGenGlobalVar *CodeGen = nullptr;

        explicit SymGlobalVar(ASTVar *GlobalVar);

    public:

        SymModule *getModule() const;

        SymComment *getComment() const;

        CodeGenGlobalVar *getCodeGen() const override;

        void setCodeGen(CodeGenGlobalVar *CG);

    };

}  // end namespace fly

#endif // FLY_SYM_GLOBALVAR_H