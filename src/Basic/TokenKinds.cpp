//===--- TokenKinds.cpp - Token Kinds Support -----------------------------===//
//
// Part of the Fly Project, under the Apache License v2.0
//
//===----------------------------------------------------------------------===//
//
//  This file implements the TokenKind enum and support functions.
//
//===----------------------------------------------------------------------===//

#include "Basic/TokenKinds.h"
#include "llvm/Support/ErrorHandling.h"
using namespace fly;

static const char * const TokNames[] = {
#define TOK(X) #X,
#define KEYWORD(X) #X,
#include "Basic/TokenKinds.def"
  nullptr
};

const char *tok::getTokenName(TokenKind Kind) {
  if (Kind < tok::NUM_TOKENS)
    return TokNames[Kind];
  llvm_unreachable("unknown TokenKind");
  return nullptr;
}

const char *tok::getPunctuatorSpelling(TokenKind Kind) {
  switch (Kind) {
#define PUNCTUATOR(X,Y) case X: return Y;
#include "Basic/TokenKinds.def"
  default: break;
  }
  return nullptr;
}

const char *tok::getKeywordSpelling(TokenKind Kind) {
  switch (Kind) {
#define KEYWORD(X) case kw_ ## X: return #X;
#include "Basic/TokenKinds.def"
    default: break;
  }
  return nullptr;
}

bool tok::isKeyword(TokenKind Kind) {
    switch (Kind) {
#define KEYWORD(X) case kw_ ## X: return true;
#include "Basic/TokenKinds.def"
        default:
            break;
    }
    return false;
}
