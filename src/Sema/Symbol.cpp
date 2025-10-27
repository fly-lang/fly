//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Symbol.cpp - Symbol implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Symbol.h"

using namespace fly;

/**
 * Symbol constructor
 */
Symbol::Symbol(std::string Name, SemaKind Kind, SemaNode* Ref) {
	this->Name = std::move(Name);
	this->Kind = Kind;
	this->Ref = Ref;
}

Symbol::Symbol(llvm::StringRef Name, SemaKind Kind, SemaNode* Ref) {
	this->Name = std::string(Name);
	this->Kind = Kind;
	this->Ref = Ref;
}

std::string Symbol::getName() const {
	return Name;
}


SemaKind Symbol::getKind() const {
	return Kind;
}


SemaNode * Symbol::getRef() const {
	return Ref;
}
