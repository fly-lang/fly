//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaSmartAlloc.cpp - Smart pointer allocation tracker
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaSmartAlloc.h"
#include "Sema/SemaCall.h"
#include "AST/ASTCall.h"

using namespace fly;

SemaSmartAlloc::SemaSmartAlloc(SemaCall *Call)
    : SemaAlloc(SemaAllocKind::SMART), Call(Call), ReferenceCounter(1) {}

SemaCall *SemaSmartAlloc::getCall() const {
    return Call;
}

bool SemaSmartAlloc::isUnique() const {
    return Call->getAST().getCallKind() == ASTCallKind::CALL_NEW_UNIQUE;
}

bool SemaSmartAlloc::isShared() const {
    return Call->getAST().getCallKind() == ASTCallKind::CALL_NEW_SHARED;
}

bool SemaSmartAlloc::isWeak() const {
    return Call->getAST().getCallKind() == ASTCallKind::CALL_NEW_WEAK;
}

uint64_t SemaSmartAlloc::getReferenceCounter() const {
    return ReferenceCounter;
}

uint64_t SemaSmartAlloc::incrReferenceCounter() {
    return ++ReferenceCounter;
}

uint64_t SemaSmartAlloc::decrReferenceCounter() {
    return --ReferenceCounter;
}
