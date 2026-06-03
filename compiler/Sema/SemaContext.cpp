//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaContext.cpp - sema context and coordination
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaContext.h"

#include "AST/ASTModule.h"
#include "Sema/Resolver.h"

#include <Basic/Debug.h>
#include <Sema/Registry.h>
#include <llvm/Support/Debug.h>

using namespace fly;

SemaContext::SemaContext(DiagnosticsEngine &Diags) : Diags(Diags), Reg(new Registry(Diags)) {

}

SemaContext::~SemaContext() {
	delete Reg;
}

llvm::SmallVector<SemaModule *, 8> SemaContext::Resolve(llvm::SmallVector<ASTModule *, 8> &Modules,
                                                          bool TestMode) {
	FLY_DEBUG_SCOPE("Sema", "Resolve");

	// Create the Resolver with AST Modules
	Resolver R(Diags, *Reg, TestMode);
	for (auto &Module : Modules) {
		Module->accept(R);
	}

	// Start the Resolution Process
	R.Resolve();
	return Reg->getModules();
}
