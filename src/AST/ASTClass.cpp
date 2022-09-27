//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClass.cpp - Class implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClass.h"
#include "AST/ASTClassField.h"
#include "AST/ASTClassMethod.h"

using namespace fly;

ASTClassScopes::ASTClassScopes(ASTClassVisibilityKind Visibility, bool Constant) :
        Visibility(Visibility), Constant(Constant){

}

ASTClassVisibilityKind ASTClassScopes::getVisibility() const {
    return Visibility;
}

bool ASTClassScopes::isConstant() const {
    return Constant;
}

std::string ASTClassScopes::str() const {
    return "ASTClassScopes {Visibility=" + std::to_string((int) Visibility) +
            ", Constant=" + std::to_string(Constant) + "}";
}

ASTClass::ASTClass(const SourceLocation &Loc, ASTNode *Node, const std::string &Name,
                   ASTTopScopes *Scopes) :
        ASTTopDef(Loc, Node, ASTTopDefKind::DEF_CLASS, Scopes), Name(Name) {

}

const std::string ASTClass::getName() const {
    return Name;
}

ASTClassKind ASTClass::getClassKind() const {
    return ClassKind;
}

llvm::StringMap<ASTClassField *> ASTClass::getFields() const {
    return Fields;
}

llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTClassMethod *, 4>>> ASTClass::getMethods() const {
    return Methods;
}

std::string ASTClass::str() const {
    // Fields to string
    std::string StrFields;
    for (auto &Field : Fields) {
        StrFields += Field.second->str() + ", ";
    }
    if (!StrFields.empty())
        StrFields.substr(StrFields.length(),StrFields.length()-1);

    // Methods to string
    std::string StrMethods;
    for (auto &MapEntry : Methods) {
        for (auto &Vector : MapEntry.second) {
            for (auto &Method :  Vector.second) {
                StrMethods += Method->str() + ", ";
            }
        }
    }
    if (!StrMethods.empty())
        StrMethods.substr(StrMethods.length(),StrMethods.length()-1);

    // Class to string
    return "ASTClass { Name=" + Name +
            ", Scopes=" + Scopes->str() +
            ", ClassKind=" + std::to_string((int) ClassKind) +
            ", Fields={ " + StrFields + " }" +
            ", Methods={" + StrMethods + " }" +
            " }";
}