//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Sema.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_RESOLVER_CLASS_H
#define FLY_SEMA_RESOLVER_CLASS_H
#include <AST/ASTFunction.h>
#include <AST/ASTVar.h>

#include "SemaComment.h"

namespace fly {

	class SemaResolver;
    class Sema;
    class SemaClassType;
    class SemaClassAttribute;
    class SemaClassMethod;
    class SemaClassInstance;

    class SemaResolverClass {

        SemaResolver *R;

        Sema &S;

        SemaClassType *Class;

    public:

        static void BaseClasses(SemaResolver *R, SemaClassType *Class);

        static void ClassDefinition(SemaResolver *R, SemaClassType *Class);

    private:

        SemaResolverClass(SemaResolver *R, SemaClassType *Class);

        void CreateDefinitions();

        SemaClassAttribute *DefineAttribute(SemaClassInstance *This, ASTVar *Var, SemaComment *Comment);

        SemaClassMethod *DefineMethod(SemaClassInstance *This, ASTFunction* Function, SemaComment * Comment);

        void CreateBaseDefinitions(SemaClassType *InheritClass);

        bool CanInheritMethod(SemaClassMethod *Method);

        bool CanInheritAttribute(SemaClassAttribute *Attribute);

        void CreateDefaultConstructor();

        void SetDefaultValueInAttributes();

        void AddBodies();
    };

} // end namespace fly

#endif