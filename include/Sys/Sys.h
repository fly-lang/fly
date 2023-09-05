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
    class ASTCall;
    class ASTVar;
    class ASTFunction;

    class Sys {

        DiagnosticsEngine &Diags;

    public:

        static void Build(ASTNameSpace *NameSpace);

        static ASTVar *getError();

        static ASTFunction *getFail0();

        static ASTFunction *getFail1();

        static ASTFunction *getFail2();

        static ASTFunction *getFail3();



    };

}

#endif // FLY_SYS_H