//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/TopDecl.h - Top declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/TopDecl.h"

using namespace fly;

TopDecl::TopDecl(const SourceLocation &Loc) : Location(Loc) {

}

const SourceLocation &TopDecl::getLocation() const {
    return Location;
}

VisibilityKind TopDecl::getVisibility() const {
    return Visibility;
}
