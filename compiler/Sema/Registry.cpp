//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/Registry.cpp - namespace registry
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Registry.h"

#include "AST/ASTCall.h"
#include "AST/ASTFunction.h"
#include "Basic/Debug.h"
#include "Basic/Diagnostic.h"
#include "llvm/Support/Signals.h"
#include "Sema/Helper.h"
#include "Sema/SemaNameSpace.h"

#include <Sema/SemaBuiltin.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaFunction.h>
#include <Sema/SemaParam.h>
#include <Sema/SymbolTable.h>

using namespace fly;

std::string Registry::DEFAULT_NAMESPACE = "default";

Registry::Registry(DiagnosticsEngine &Diags) : Diags(Diags),
	BuiltinScope(CreateBuiltinScope()),
	GlobalScope(new SymbolTable(BuiltinScope)),
	DefaultNameSpace(new SemaNameSpace(DEFAULT_NAMESPACE, new SymbolTable(GlobalScope))) {
	GlobalScope->insert(new Symbol(DefaultNameSpace->getName(), SymbolKind::NAMESPACE, DefaultNameSpace));
}

Registry::~Registry() {
	for (auto *M : Modules)
		delete M;

	GlobalScope->deleteChildren();
	delete GlobalScope;

	delete DefaultNameSpace;

	BuiltinScope->deleteChildren();
	delete BuiltinScope;
}

DiagnosticBuilder Registry::Diag(const SourceLocation &Loc, unsigned DiagID) const {
	return Diags.Report(Loc, DiagID);
}

DiagnosticBuilder Registry::Diag(unsigned DiagID) const {
	if (DebugLog && DiagID == diag::err_invalid_behavior)
		llvm::sys::PrintStackTrace(llvm::errs());
	return Diags.Report(DiagID);
}

SymbolTable* Registry::CreateBuiltinScope() {
	SymbolTable* Builtin = new SymbolTable(nullptr);

	auto BoolType = SemaBuiltin::getBoolType();
	auto ByteType = SemaBuiltin::getByteType();
	auto UShortType = SemaBuiltin::getUShortType();
	auto ShortType = SemaBuiltin::getShortType();
	auto UIntType = SemaBuiltin::getUIntType();
	auto IntType = SemaBuiltin::getIntType();
	auto ULongType = SemaBuiltin::getULongType();
	auto LongType = SemaBuiltin::getLongType();
	auto FloatType = SemaBuiltin::getFloatType();
	auto DoubleType = SemaBuiltin::getDoubleType();
	auto StringType = SemaBuiltin::getStringType();
	auto VoidType = SemaBuiltin::getVoidType();
	auto ErrorType = SemaBuiltin::getErrorType();

	// Insert Builtin Types
	Builtin->insert(new Symbol(BoolType->getName(), SymbolKind::BUILTIN_TYPE, BoolType));
	Builtin->insert(new Symbol(ByteType->getName(), SymbolKind::BUILTIN_TYPE, ByteType));
	Builtin->insert(new Symbol(UShortType->getName(), SymbolKind::BUILTIN_TYPE, UShortType));
	Builtin->insert(new Symbol(ShortType->getName(), SymbolKind::BUILTIN_TYPE, ShortType));
	Builtin->insert(new Symbol(UIntType->getName(), SymbolKind::BUILTIN_TYPE, UIntType));
	Builtin->insert(new Symbol(IntType->getName(), SymbolKind::BUILTIN_TYPE, IntType));
	Builtin->insert(new Symbol(ULongType->getName(), SymbolKind::BUILTIN_TYPE, ULongType));
	Builtin->insert(new Symbol(LongType->getName(), SymbolKind::BUILTIN_TYPE, LongType));
	Builtin->insert(new Symbol(FloatType->getName(), SymbolKind::BUILTIN_TYPE, FloatType));
	Builtin->insert(new Symbol(DoubleType->getName(), SymbolKind::BUILTIN_TYPE, DoubleType));
	Builtin->insert(new Symbol(StringType->getName(), SymbolKind::BUILTIN_TYPE, StringType));
	Builtin->insert(new Symbol(VoidType->getName(), SymbolKind::BUILTIN_TYPE, VoidType));
	Builtin->insert(new Symbol(ErrorType->getName(), SymbolKind::BUILTIN_TYPE, ErrorType));

	return Builtin;
}

void Registry::addModule(SemaModule *Module) {
	Modules.push_back(Module);
}

llvm::SmallVector<SemaModule *, 8> &Registry::getModules() {
	return Modules;
}

SemaNameSpace * Registry::getDefaultNameSpace() {
	return DefaultNameSpace;
}

llvm::SmallVector<SemaFunctionBase *, 4> Registry::getBodies() const {
	return Bodies;
}

void Registry::addBody(SemaFunctionBase *FunctionBase) {
	Bodies.push_back(FunctionBase);
}

SemaNameSpace* Registry::getOrCreateNameSpace(const llvm::SmallVector<ASTName *, 4>& Names) {
	if (Names.empty())
		return DefaultNameSpace;

	// Define working variables
	SemaNameSpace *NameSpace = nullptr;
	SymbolTable *CurrentScope = GlobalScope;

	// Iterate through names
	for (auto *N : Names) {
		llvm::StringRef Name = N->getName();

		// Lookup in current scope
		llvm::SmallVector<Symbol *, 8> *Symbols = CurrentScope->lookup(Name);

		// Create namespace if not found
		if (Symbols == nullptr) {
			SymbolTable *ChildScope = new SymbolTable(CurrentScope);
			NameSpace = new SemaNameSpace(Name, ChildScope);
			CurrentScope->insert(new Symbol(NameSpace->getName(), SymbolKind::NAMESPACE, NameSpace));
			CurrentScope->addChild(ChildScope);
			CurrentScope = ChildScope; // descend into the new namespace for subsequent names
			continue;
		}

		// Symbol Name conflict in NameSpace
		if (Symbols->size() > 1) {
			// Error:
			Diag(diag::err_invalid_behavior);
		}

		// If symbol is a NameSpace the search may be deeper
		Symbol *CurrentSymbol = (*Symbols)[0];
		if (CurrentSymbol->getKind() != SymbolKind::NAMESPACE) {
			// Error:
			Diag(diag::err_invalid_behavior);
		}

		// Take the NameSpace
		NameSpace = static_cast<SemaNameSpace *>(CurrentSymbol->getRef());
		CurrentScope = NameSpace->getSymbols();
	}

	return NameSpace;
}

Symbol *Registry::LookupImport(const llvm::SmallVector<ASTName *, 4> &Names) {
	Symbol *CurrentSymbol = nullptr;
	SymbolTable *Scope = GlobalScope;

	for (int i = 0; i < Names.size(); i++) {
		llvm::StringRef Name = Names[i]->getName();

		// Lookup Name in current Scope
		llvm::SmallVector<Symbol *, 8> *Symbols = Scope->lookup(Name);

		if (!Symbols) {
			// Error: Symbol not found
			llvm::errs() << "[DEBUG] LookupImport: symbol not found: '" << Name << "' at index " << i << "\n";
			Diag(diag::err_invalid_behavior);
			return nullptr;
		}

		if (Symbols->size() > 1) {
			// Error: Symbol Name conflict
			llvm::errs() << "[DEBUG] LookupImport: symbol conflict (size=" << Symbols->size() << "): '" << Name << "' at index " << i << "\n";
			Diag(diag::err_invalid_behavior);
			return nullptr;
		}

		// Take the unique Symbol
		CurrentSymbol = (*Symbols)[0];

		// Symbol may be: NameSpace, Type, Function ...
		// If symbol is a NameSpace the search may be deeper
		if (CurrentSymbol->getKind() == SymbolKind::NAMESPACE) {
			Scope = static_cast<SemaNameSpace *>(CurrentSymbol->getRef())->getSymbols();
		}
	}

	return CurrentSymbol;
}

Symbol* Registry::LookupBuiltinType(llvm::StringRef TypeName) {
	SmallVector<Symbol *, 8> *Symbols = BuiltinScope->lookup(TypeName);
	if (!Symbols || Symbols->empty()) {
		Diag(diag::err_invalid_behavior);
		return nullptr;
	}
	return (*Symbols)[0];
}

Symbol *Registry::LookupNamedType(llvm::StringRef Name, SymbolTable *Scope) {
	// Lookup Name in current Scope
	llvm::SmallVector<Symbol *, 8> *Symbols = Scope->lookupInParents(Name);

	if (!Symbols) {
		// Error: Symbol not found
		Diag(diag::err_invalid_behavior);
		return nullptr;
	}

	if (Symbols->size() > 1) {
		// Error: Symbol Name conflict
		Diag(diag::err_invalid_behavior);
		return nullptr;
	}

	// Take the unique Symbol
	Symbol *CurrentSymbol = (*Symbols)[0];

	if (CurrentSymbol->getKind() == SymbolKind::BUILTIN_TYPE) {
		// Error: Symbol is not a Type
		Diag(diag::err_invalid_behavior);
	}

	return CurrentSymbol;
}

Symbol *Registry::LookupNamedType(ASTNamedType &NamedType, SymbolTable *Scope) {
	const SmallVector<ASTName *, 4> &Names = NamedType.getNames();
	SymbolTable *CurrentScope = Scope;
	Symbol *CurrentSymbol = nullptr;

	for (int i = 0; i < Names.size(); i++) {
		llvm::StringRef Name = Names[i]->getName();

		// Lookup Name in current Scope
		llvm::SmallVector<Symbol *, 8> *Symbols = CurrentScope->lookupInParents(Name);

		if (!Symbols) {
			// Error: Symbol not found
			Diag(NamedType.getLocation(), diag::err_sema_unknown_type) << Helper::Flatten(Names);
			return nullptr;
		}

		if (Symbols->size() > 1) {
			// Error: Symbol Name conflict
			Diag(NamedType.getLocation(), diag::err_sema_multiple_definition) << Helper::Flatten(Names);
			return nullptr;
		}

		// Take the unique Symbol
		CurrentSymbol = (*Symbols)[0];

		// If symbol is a NameSpace the search may be deeper
		if (CurrentSymbol->getKind() == SymbolKind::NAMESPACE) {
			CurrentScope = static_cast<SemaNameSpace *>(CurrentSymbol->getRef())->getSymbols();
		}
	}

	if (CurrentSymbol->getKind() == SymbolKind::BUILTIN_TYPE ||
		CurrentSymbol->getKind() == SymbolKind::CLASS ||
		CurrentSymbol->getKind() == SymbolKind::ENUM) {
		// Return Type
		return CurrentSymbol;
	}

	// Error: Symbol is not a Type or NameSpace
	Diag(diag::err_invalid_behavior);
	return nullptr;
}

//
// SemaNameSpace* Registry::LookupNameSpace(llvm::StringRef Name, SymbolTable *Scope) {
// 	if (Scope == nullptr) Scope = GlobalScope;
//
// 	llvm::SmallVector<Symbol *, 8> *Symbols = Scope->lookup(Name);
//
// 	if (!Symbols) {
// 		// Error: Symbol not found
// 		Diag(diag::err_invalid_behavior);
// 		return nullptr;
// 	}
//
// 	if (Symbols->size() > 1) {
// 		// Error: Symbol Name conflict
// 		Diag(diag::err_invalid_behavior);
// 		return nullptr;
// 	}
//
// 	// Take the unique Symbol
// 	Symbol *Sym = (*Symbols)[0];
//
// 	if (Sym->getKind() != SemaKind::NAMESPACE) {
// 		// Error: Symbol is not a NameSpace
// 		Diag(diag::err_invalid_behavior);
// 		return nullptr;
// 	}
//
// 	return static_cast<SemaNameSpace *>(Sym->getRef());
// }


Symbol *Registry::LookupName(llvm::StringRef Name, SymbolTable *Scope) {
	if (Scope == nullptr) Scope = GlobalScope;

	llvm::SmallVector<Symbol *, 8> *Symbols = Scope->lookup(Name);

	if (!Symbols) {
		// Error: Symbol not found
		Diag(diag::err_invalid_behavior);
		return nullptr;
	}

	if (Symbols->size() > 1) {
		// Error: Symbol Name conflict
		Diag(diag::err_invalid_behavior);
		return nullptr;
	}

	// Take the unique Symbol
	return (*Symbols)[0];
}

Symbol *Registry::LookupFunction(llvm::StringRef Name, SmallVector<SemaType *, 8> &Types, SymbolTable *Scope) {
	if (Scope == nullptr) Scope = GlobalScope;
	llvm::SmallVector<Symbol *, 8> *Symbols = Scope->lookupInParents(Name);

	if (!Symbols) {
		return nullptr;
	}

	// Iterate through all symbols with this name to find the right function
	for (Symbol *Sym : *Symbols) {

		// Check if symbol is a function
		if (Sym->getKind() != SymbolKind::FUNCTION) {
			continue;
		}

		SemaFunctionBase *Function = static_cast<SemaFunctionBase *>(Sym->getRef());
		llvm::SmallVector<SemaParam *, 8> &Params = Function->getParams();

		// Check if the number of parameters matches
		if (Params.size() != Types.size()) {
			continue;
		}

		// Check if all parameter types match
		bool AllTypesMatch = true;
		for (size_t i = 0; i < Params.size(); i++) {
			SemaType *ParamType = Params[i]->getType();
			SemaType *ArgType = Types[i];

			// Direct type match using isEquals method
			if (ParamType->isEquals(ArgType)) {
				continue;
			}

			// Check class inheritance: if both are class types, check if ArgType is derived from ParamType
			if (ParamType->isClass() && ArgType->isClass()) {
				SemaClassType *ParamClassType = static_cast<SemaClassType *>(ParamType);
				SemaClassType *ArgClassType = static_cast<SemaClassType *>(ArgType);

				// Check if ArgType is derived from or equals ParamType
				if (ArgClassType->isDerivedOrEquals(ParamClassType)) {
					continue;
				}
			}

			// Check numeric type compatibility: smaller numeric types can be implicitly promoted
			if (ParamType->isNumber() && ArgType->isNumber()) {
				continue;
			}

			// Types don't match
			AllTypesMatch = false;
			break;
		}

		// If all types match, return this function
		if (AllTypesMatch) {
			return Sym;
		}
	}

	return nullptr;
}

Symbol *Registry::LookupFunctionExact(llvm::StringRef Name, SmallVector<SemaType *, 8> &Types, SymbolTable *Scope) {
	if (Scope == nullptr) Scope = GlobalScope;
	llvm::SmallVector<Symbol *, 8> *Symbols = Scope->lookupInParents(Name);
	if (!Symbols) return nullptr;

	for (Symbol *Sym : *Symbols) {
		if (Sym->getKind() != SymbolKind::FUNCTION) continue;

		SemaFunctionBase *Function = static_cast<SemaFunctionBase *>(Sym->getRef());
		llvm::SmallVector<SemaParam *, 8> &Params = Function->getParams();

		if (Params.size() != Types.size()) continue;

		bool AllMatch = true;
		for (size_t i = 0; i < Params.size(); i++) {
			if (!Params[i]->getType()->isEquals(Types[i])) {
				AllMatch = false;
				break;
			}
		}
		if (AllMatch) return Sym;
	}
	return nullptr;
}

static bool FunctionTypesMatchExact(SemaFunctionBase *Function, SmallVector<SemaType *, 8> &Types) {
	llvm::SmallVector<SemaParam *, 8> &Params = Function->getParams();
	if (Params.size() != Types.size()) return false;
	for (size_t i = 0; i < Params.size(); i++) {
		SemaType *ParamType = Params[i]->getType();
		SemaType *ArgType = Types[i];
		if (ParamType->isEquals(ArgType)) continue;
		if (ParamType->isClass() && ArgType->isClass()) {
			if (static_cast<SemaClassType *>(ArgType)->isDerivedOrEquals(
			        static_cast<SemaClassType *>(ParamType))) continue;
		}
		return false;
	}
	return true;
}

static bool FunctionTypesMatch(SemaFunctionBase *Function, SmallVector<SemaType *, 8> &Types) {
	llvm::SmallVector<SemaParam *, 8> &Params = Function->getParams();
	if (Params.size() != Types.size()) return false;
	for (size_t i = 0; i < Params.size(); i++) {
		SemaType *ParamType = Params[i]->getType();
		SemaType *ArgType = Types[i];
		if (ParamType->isEquals(ArgType)) continue;
		if (ParamType->isClass() && ArgType->isClass()) {
			if (static_cast<SemaClassType *>(ArgType)->isDerivedOrEquals(
			        static_cast<SemaClassType *>(ParamType))) continue;
		}
		if (ParamType->isNumber() && ArgType->isNumber()) continue;
		return false;
	}
	return true;
}

llvm::SmallVector<Symbol *, 4> Registry::FindFunctionCandidates(llvm::StringRef Name, SymbolTable *Scope) {
	if (Scope == nullptr) Scope = GlobalScope;
	llvm::SmallVector<Symbol *, 4> Result;
	llvm::SmallVector<Symbol *, 8> *Symbols = Scope->lookupInParents(Name);
	if (!Symbols) return Result;
	for (Symbol *Sym : *Symbols) {
		if (Sym->getKind() == SymbolKind::FUNCTION)
			Result.push_back(Sym);
	}
	return Result;
}

llvm::SmallVector<Symbol *, 4> Registry::FindFunctionMatches(llvm::StringRef Name,
                                                              SmallVector<SemaType *, 8> &Types,
                                                              SymbolTable *Scope) {
	if (Scope == nullptr) Scope = GlobalScope;
	llvm::SmallVector<Symbol *, 8> *Symbols = Scope->lookupInParents(Name);
	if (!Symbols) return {};

	// First pass: prefer exact matches (no numeric promotion) to avoid ambiguity
	// when multiple overloads differ only in numeric type (e.g. int vs long).
	llvm::SmallVector<Symbol *, 4> ExactResult;
	for (Symbol *Sym : *Symbols) {
		if (Sym->getKind() != SymbolKind::FUNCTION) continue;
		if (FunctionTypesMatchExact(static_cast<SemaFunctionBase *>(Sym->getRef()), Types))
			ExactResult.push_back(Sym);
	}
	if (!ExactResult.empty()) return ExactResult;

	// Second pass: fall back to promotion-aware matches.
	llvm::SmallVector<Symbol *, 4> Result;
	for (Symbol *Sym : *Symbols) {
		if (Sym->getKind() != SymbolKind::FUNCTION) continue;
		if (FunctionTypesMatch(static_cast<SemaFunctionBase *>(Sym->getRef()), Types))
			Result.push_back(Sym);
	}
	return Result;
}
