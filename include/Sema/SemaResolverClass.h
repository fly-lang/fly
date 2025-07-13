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
#include "SemaType.h"

namespace fly {

	class SemaResolver;
    class Sema;
    class SemaClassType;

    class SemaResolverClass {

        SemaResolver *R;

        Sema &S;

        SemaClassType *Class;

    public:

        static void Resolve(SemaResolver *R, SemaClassType *Class);

    private:

        SemaResolverClass(SemaResolver *R, SemaClassType *Class);

        void Extends();

        void InheritMethods(fly::SemaClassType* ClassType);

        void InheritAttributes(fly::SemaClassType* ClassType);

        void Definitions();

        void CreateDefaultConstructor();

        void SetDefaultValueInAttributes();

        void AddBodies();
    };

} // end namespace fly

#endif