//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentityType.cpp - Type implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTIdentityType.h"
#include "AST/ASTIdentity.h"
#include "AST/ASTIdentifier.h"

using namespace fly;

ASTIdentityTypeKind toIdentityKind(ASTTopDefKind Kind) {
    if (Kind == fly::ASTTopDefKind::DEF_CLASS) {
        return ASTIdentityTypeKind::TYPE_CLASS;
    }
    if (Kind == fly::ASTTopDefKind::DEF_ENUM) {
        return ASTIdentityTypeKind::TYPE_ENUM;
    }
    return ASTIdentityTypeKind::TYPE_NONE;
}

ASTIdentityType::ASTIdentityType(ASTIdentifier *Identifier) :
        ASTType(Identifier->getLocation(), ASTTypeKind::TYPE_IDENTITY),
        ASTIdentifier(Identifier->getLocation(), Identifier->getName(), ASTIdentifierKind::REF_TYPE), IdentityKind(ASTIdentityTypeKind::TYPE_NONE) {

}

ASTIdentityType::ASTIdentityType(ASTIdentifier *Identifier, ASTIdentityTypeKind IdentityKind) :
        ASTType(Identifier->getLocation(), ASTTypeKind::TYPE_IDENTITY),
        ASTIdentifier(Identifier->getLocation(), Identifier->getName(), ASTIdentifierKind::REF_TYPE), IdentityKind(IdentityKind) {

}

ASTIdentityType::ASTIdentityType(ASTIdentity *Def) :
        ASTType(SourceLocation(), ASTTypeKind::TYPE_IDENTITY),
        ASTIdentifier(SourceLocation(), Def->getName(), ASTIdentifierKind::REF_TYPE),
        Def(Def), IdentityKind(toIdentityKind(Def->getTopDefKind())) {

}

SourceLocation ASTIdentityType::getLocation() const {
    return ASTIdentifier::getLocation();
}

ASTIdentity *ASTIdentityType::getDef() const {
    return Def;
}

ASTIdentityTypeKind ASTIdentityType::getIdentityKind() const {
    return IdentityKind;
}

bool ASTIdentityType::operator ==(const ASTIdentityType &IdentityType) const {
    return IdentityType.getIdentityKind() != ASTIdentityTypeKind::TYPE_NONE &&
            IdentityType.getIdentityKind() == getIdentityKind() && IdentityType.getName() == getName();
}

const bool ASTIdentityType::isNone() const {
    return IdentityKind == ASTIdentityTypeKind::TYPE_NONE;
}

const bool ASTIdentityType::isClass() const {
    return IdentityKind == ASTIdentityTypeKind::TYPE_CLASS;
}

const bool ASTIdentityType::isEnum() const {
    return IdentityKind == ASTIdentityTypeKind::TYPE_ENUM;
}

const std::string ASTIdentityType::print() const {
    return Name.data();
}

std::string ASTIdentityType::str() const {
    return Logger("ASTClassType").
    Super(ASTType::str()).
    Attr("Def", (Debuggable *) Def).
    End();
}
