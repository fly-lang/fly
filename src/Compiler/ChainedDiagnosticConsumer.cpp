//===----------------------------------------------------------------------===//
// Compiler/ChainedDiagnosticConsumer.cpp - Chain Diagnostic Clients
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#include "Compiler/ChainedDiagnosticConsumer.h"

using namespace fly;

void ChainedDiagnosticConsumer::anchor() { }
