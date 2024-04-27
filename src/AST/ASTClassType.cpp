//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTClassType.cpp - Class Type implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClassType.h"

using namespace fly;

ASTClassType::ASTClassType(ASTIdentifier *Identifier) : ASTIdentityType(Identifier, ASTIdentityTypeKind::TYPE_CLASS) {

}

ASTClassType::ASTClassType(ASTClass *Def) : ASTIdentityType((ASTIdentity *) Def) {

}

ASTIdentity *ASTClassType::getDef() const {
    return Def;
}
