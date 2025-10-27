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

    class ASTClass;
	class Resolver;
    class Sema;
    class SemaClassType;
    class SemaClassAttribute;
    class SemaClassMethod;
    class SemaClassInstance;

    class SemaResolverClass {

        Resolver *R;

        Sema &S;

        SemaClassType *Class;

    public:

        SemaResolverClass(Resolver *R, SemaClassType *Class);

        void Resolve();

    private:

        void ResolveDefinitions();

        SemaClassAttribute *DefineAttribute(ASTVar *Var, SemaComment *Comment);

        SemaClassMethod *DefineMethod(SemaClassInstance *This, ASTFunction* Function, SemaComment * Comment);

        void ResolveBaseClasses(SemaClassType *DerivedClass);

        bool CanInheritMethod(SemaClassMethod *Method);

        bool CanInheritAttribute(SemaClassAttribute *Attribute);

        void CreateDefaultConstructor();

        void SetDefaultValueInAttributes();

        void AddBodies();
    };

} // end namespace fly

#endif