//===--------------------------------------------------------------------------------------------------------------===//
// include/Sys/Sys.h - System Fail Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYS_H
#define FLY_SYS_H

#include "AST/ASTNameSpace.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/SmallVector.h"

#include <map>

namespace fly {

    class DiagnosticsEngine;
    class Sema;
    class ASTCall;
    class ASTVar;
    class ASTFunction;

    class Sys {

        Sema &S;

    public:

        static void Build(Sema &S);

        Sys(Sema &S);

        void AddFailFunctions();

        ASTVar *getError();

        ASTFunction *getFail0();

        ASTFunction *getFail1();

        ASTFunction *getFail2();

        ASTFunction *getFail3();



    };

}

#endif // FLY_SYS_H