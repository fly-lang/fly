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
    Identifier(Identifier), Kind(ASTIdentityTypeKind::TYPE_NONE) {

}

ASTIdentityType::ASTIdentityType(ASTIdentifier *Identifier, ASTIdentityTypeKind IdentityKind) :
    ASTType(Identifier->getLocation(), ASTTypeKind::TYPE_IDENTITY),
    Identifier(Identifier), Kind(IdentityKind) {

}

ASTIdentityType::ASTIdentityType(ASTIdentity *Def) :
    ASTType(SourceLocation(), ASTTypeKind::TYPE_IDENTITY),
    Def(Def), Kind(toIdentityKind(Def->getKind())) {

}

SourceLocation ASTIdentityType::getLocation() const {
    return Identifier ? Identifier->getLocation() : SourceLocation();
}

llvm::StringRef ASTIdentityType::getName() const {
    return Identifier ? Identifier->getName() : llvm::StringRef();
}

ASTIdentifier *ASTIdentityType::getIdentifier() const {
    return Identifier;
}

ASTIdentity *ASTIdentityType::getDef() const {
    return Def;
}

ASTIdentityTypeKind ASTIdentityType::getKind() const {
    return Kind;
}

bool ASTIdentityType::operator ==(const ASTIdentityType &IdentityType) const {
    return IdentityType.getKind() != ASTIdentityTypeKind::TYPE_NONE &&
        IdentityType.getKind() == getKind() && IdentityType.getName() == getName();
}

const bool ASTIdentityType::isClass() const {
    return Kind == ASTIdentityTypeKind::TYPE_CLASS;
}

const bool ASTIdentityType::isEnum() const {
    return Kind == ASTIdentityTypeKind::TYPE_ENUM;
}

const std::string ASTIdentityType::print() const {
    return Def ? Def->print() : Identifier->print();
}

std::string ASTIdentityType::str() const {
    return Logger("ASTClassType").
    Super(ASTType::str()).
    Attr("Identifier", Identifier).
    Attr("Def", (Debuggable *) Def).
    End();
}
