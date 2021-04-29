//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/Refer.h - Reference to other
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_REFER_H
#define FLY_REFER_H

#include "Decl.h"

namespace fly {

    class Refer {

        DeclBase *Ref = NULL;

        virtual DeclBase *getRef() { return Ref;}

    };
}

#endif //FLY_REFER_H
