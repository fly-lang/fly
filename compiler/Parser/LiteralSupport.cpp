//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Parser/LiteralSupport.cpp - literal token parsing
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/LiteralSupport.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/ConvertUTF.h"
#include <cassert>
#include <cstdint>

using namespace fly;
using namespace llvm;

static void appendCodePoint(unsigned Codepoint,
                            llvm::SmallVectorImpl<char> &Str) {
  char ResultBuf[4];
  char *ResultPtr = ResultBuf;
  bool Res = llvm::ConvertCodePointToUTF8(Codepoint, ResultPtr);
  (void)Res;
  assert(Res && "Unexpected conversion failure");
  Str.append(ResultBuf, ResultPtr);
}

void fly::expandUCNs(SmallVectorImpl<char> &Buf, StringRef Input) {
  for (StringRef::iterator I = Input.begin(), E = Input.end(); I != E; ++I) {
    if (*I != '\\') {
      Buf.push_back(*I);
      continue;
    }

    ++I;
    assert(*I == 'u' || *I == 'U');

    unsigned NumHexDigits;
    if (*I == 'u')
      NumHexDigits = 4;
    else
      NumHexDigits = 8;

    assert(I + NumHexDigits <= E);

    uint32_t CodePoint = 0;
    for (++I; NumHexDigits != 0; ++I, --NumHexDigits) {
      unsigned Value = llvm::hexDigitValue(*I);
      assert(Value != -1U);

      CodePoint <<= 4;
      CodePoint += Value;
    }

    appendCodePoint(CodePoint, Buf);
    --I;
  }
}
