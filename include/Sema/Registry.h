//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Registry.h - Module Manager
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_NAMESPACE_REGISTRY_H
#define FLY_NAMESPACE_REGISTRY_H

#include <string>
#include <unordered_map>
#include <AST/ASTName.h>
#include <llvm/ADT/SmallVector.h>

#include "Resolver.h"

namespace fly {

	class SemaNameSpace;
	class SemaModule;
	class ASTModule;
	class ASTBlockStmt;
	class Symbol;

	struct LocalScope {
		SemaFunctionBase* Function;
		SymbolTable* Symbols;
	};

	class Registry {

		DiagnosticsEngine &Diags;

		SymbolTable *BuiltinScope;

		SymbolTable* GlobalScope;

		static std::string DEFAULT_NAMESPACE;

		llvm::SmallVector<SemaModule *, 8> Modules;
		
		SemaNameSpace *DefaultNameSpace;

		llvm::SmallVector<SemaFunctionBase *, 4> Bodies;

		// Diagnostics
		DiagnosticBuilder Diag(const SourceLocation &Loc, unsigned DiagID) const;
		DiagnosticBuilder Diag(unsigned DiagID) const;

	public:

		Registry(DiagnosticsEngine &Diags);

		~Registry();

		SymbolTable * CreateBuiltinScope();

		void addModule(SemaModule* Module);

		llvm::SmallVector<SemaModule *, 8> &getModules();

		SemaNameSpace * getDefaultNameSpace();

		SemaNameSpace* getOrCreateNameSpace(const llvm::SmallVector<ASTName *, 4>& Names);

		Symbol *LookupImport(const llvm::SmallVector<ASTName *, 4> &Names);

		Symbol *LookupBuiltinType(llvm::StringRef TypeName);

		Symbol *LookupNamedType(llvm::StringRef Name, SymbolTable *Scope);

		Symbol *LookupNamedType(ASTNamedType &NamedType, SymbolTable *Scope);

		Symbol *LookupName(llvm::StringRef Name, SymbolTable *Scope = nullptr);

		Symbol *LookupFunction(llvm::StringRef Name, SmallVector<SemaType *, 8> &Types, SymbolTable *Scope);

		llvm::SmallVector<SemaFunctionBase *, 4> getBodies() const;

		void addBody(SemaFunctionBase *FunctionBase);

	};

}

#endif //FLY_NAMESPACE_REGISTRY_H
