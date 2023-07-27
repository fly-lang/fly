//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTEnumType.cpp - Type implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTEnumType.h"

using namespace fly;

ASTEnumType::ASTEnumType(ASTIdentifier *Identifier) : ASTIdentityType(Identifier, ASTIdentityTypeKind::TYPE_ENUM) {

}

ASTEnumType::ASTEnumType(ASTEnum *Def) : ASTIdentityType((ASTIdentity *) Def) {

}

ASTIdentity *ASTEnumType::getDef() const {
    return Def;
}