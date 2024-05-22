//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTClassFuntion.h - AST Class Method header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_AST_CLASSFUNCTION_H
#define FLY_AST_CLASSFUNCTION_H

#include "ASTFunctionBase.h"
#include "CodeGen/CodeGenClassFunction.h"

namespace fly {

    class ASTClass;
    class ASTScopes;
    class ASTType;
    class ASTFunction;
    class ASTScopes;

    enum class ASTClassMethodKind {
        METHOD_CUSTOM,
        METHOD_CONSTRUCTOR,
        METHOD_STATIC,
        METHOD_VIRTUAL
    };

    class ASTClassMethod : public ASTFunctionBase {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        llvm::StringRef Name;

        ASTClassMethodKind MethodKind;

        ASTClass *Class = nullptr;

        ASTClass *DerivedClass = nullptr;

        llvm::StringRef Comment;

        // Populated during codegen phase
        CodeGenClassFunction *CodeGen = nullptr;

        ASTClassMethod(const SourceLocation &Loc, ASTClassMethodKind MethodKind, ASTType *Type,
                       llvm::StringRef Name, ASTScopes *Scopes);

    public:

        const StringRef &getName() const;

        ASTClassMethodKind getMethodKind() const;

        ASTClass *getClass() const;

        ASTClass *getDerivedClass() const;

        bool isConstructor();

        bool isStatic();

        bool isAbstract() const;

        CodeGenClassFunction *getCodeGen() const override;

        void setCodeGen(CodeGenClassFunction *CGCF);

        std::string str() const override;

    };
}

#endif //FLY_AST_CLASSFUNCTION_H
