//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaFunction.h - function semantic analysis
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

        SemaModule *Module = nullptr;

        SemaComment *Comment = nullptr;

        SemaVisibilityKind Visibility;

        llvm::SmallVector<SemaLocalVar *, 4> LocalVars;

        // Generic function support (monomorphization)
        llvm::SmallVector<SemaTypeParam *, 4> TypeParams; // non-empty → generic template
        llvm::StringMap<SemaFunction *> Specializations;  // mangled key → concrete clone
        SemaFunction *GenericTemplate = nullptr;          // non-null for specializations
        std::string MangledName;                          // set for specializations

        // Populated during codegen phase
        CodeGenFunction *CodeGen = nullptr;

        explicit SemaFunction(ASTFunction &AST, SymbolTable *Scope);

    public:

        ~SemaFunction() override;

        SemaModule *getModule() const;

        SemaComment *getComment() const;

        SemaVisibilityKind getVisibility() const;

    	void setVisibility(SemaVisibilityKind Visibility);

        const llvm::SmallVector<SemaLocalVar *, 4> &getLocalVars() const;

        // Generic function support
        const llvm::SmallVector<SemaTypeParam *, 4> &getTypeParams() const { return TypeParams; }
        bool isGeneric() const { return !TypeParams.empty() && GenericTemplate == nullptr; }
        SemaFunction *getGenericTemplate() const { return GenericTemplate; }
        llvm::StringMap<SemaFunction *> &getSpecializations() { return Specializations; }

        llvm::StringRef getName() const;

        CodeGenFunction *getCodeGen() const override;

        void setCodeGen(CodeGenFunction *CGF);

        std::string str() const override;

        void accept(SemaVisitor& Visitor) override;
    };

}  // end namespace fly

#endif // FLY_SEMA_FUNCTION_H