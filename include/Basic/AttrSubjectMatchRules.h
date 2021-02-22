//===-- AttrSubjectMatchRules.h - Attribute subject match rules -*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_ATTR_SUBJECT_MATCH_RULES_H
#define LLVM_FLY_BASIC_ATTR_SUBJECT_MATCH_RULES_H

#include "Basic/SourceLocation.h"
#include "llvm/ADT/DenseMap.h"

namespace fly {
namespace attr {

/// A list of all the recognized kinds of attributes.
enum SubjectMatchRule {
#define ATTR_MATCH_RULE(X, Spelling, IsAbstract) X,
#include "Basic/AttrSubMatchRulesList.inc"
};

const char *getSubjectMatchRuleSpelling(SubjectMatchRule Rule);

using ParsedSubjectMatchRuleSet = llvm::DenseMap<int, SourceRange>;

} // end namespace attr
} // end namespace clang

#endif
