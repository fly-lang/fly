//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Sema.cpp - The Sema
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Sema.h"
#include "Sema/Resolver.h"
#include "AST/ASTModule.h"

#include <Sema/Registry.h>

using namespace fly;

Sema::Sema(DiagnosticsEngine &Diags) : Diags(Diags) {

}

llvm::SmallVector<SemaModule *, 8> Sema::Resolve(llvm::SmallVector<ASTModule *, 8> &ASTModules) {
	// Initialize the Registry
	Registry *Reg = new Registry();

	// Create the Resolver with AST Modules
	Resolver R(Diags, *Reg);
	for (auto &Module : ASTModules) {
		Module->accept(R);
	}

	// Start the Resolution Process
	R.Resolve();

	return Reg->getModules();
}
