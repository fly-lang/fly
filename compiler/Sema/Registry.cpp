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
#include <Sema/SemaType.h>
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
	auto PtrSizeType = SemaBuiltin::getPtrSizeType();
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
	Builtin->insert(new Symbol(PtrSizeType->getName(), SymbolKind::BUILTIN_TYPE, PtrSizeType));
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

const llvm::SmallVector<SemaFunctionBase *, 4> &Registry::getBodies() const {
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
			// Import target namespace component not found — the source files for that
			// namespace are not part of this compilation unit.  Return null silently
			// so ResolveImports can emit a proper "namespace not found" diagnostic
			// instead of a fatal internal error.
			return nullptr;
		}

		if (Symbols->size() > 1) {
			// Error: Symbol Name conflict
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
	// Use lookupTypeInParents to skip constructor/function symbols in the class's own scope
	// when looking for a type with the same name (e.g. new Node() inside Node methods).
	llvm::SmallVector<Symbol *, 8> *Symbols = Scope->lookupTypeInParents(Name);

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

Symbol *Registry::LookupNamedType(ASTNamedType &NamedType, SymbolTable *Scope, bool SuppressError) {
	const SmallVector<ASTName *, 4> &Names = NamedType.getNames();
	SymbolTable *CurrentScope = Scope;
	Symbol *CurrentSymbol = nullptr;

	for (int i = 0; i < Names.size(); i++) {
		llvm::StringRef Name = Names[i]->getName();

		// For the last name in a qualified path, use lookupTypeInParents so that a
		// constructor symbol with the same name in the class's own scope does not shadow
		// the class type in an outer (module/namespace) scope.
		bool IsLastName = (i == (int)Names.size() - 1);
		llvm::SmallVector<Symbol *, 8> *Symbols = IsLastName
		    ? CurrentScope->lookupTypeInParents(Name)
		    : CurrentScope->lookupInParents(Name);

		if (!Symbols) {
			if (!SuppressError)
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

// Number of explicit (user-visible) parameters: excludes the hidden output
// params the Resolver appended for the return convention — ONE 'out' for a
// single return, __out_0..N-1 for a multi-return. The old ReturnType-based
// check stripped only the single 'out' (a multi-return function keeps
// ReturnType null), so every multi-return call failed the arity match and
// reported "no overload accepts these arguments". Synthetics are recognized
// by the SemaParam flag the Resolver stamps at synthesis time — matching the
// NAME instead stripped USER params that happen to be called `out` (the
// runtime convention: strSize, mem_alloc, …), breaking every call to them.
static size_t ExplicitParamCount(SemaFunctionBase *Function) {
	auto &Params = Function->getParams();
	size_t N = Params.size();
	while (N > 0 && Params[N - 1]->isSynthetic())
		--N;
	return N;
}

// Decide whether a call passing NArgs arguments can target this function, and
// through how many leading params it must be type-checked (Count).
//
// TWO arities are legal for the same callee. `divmod(17, 5)` passes only the
// EXPLICIT params and lets the Resolver synthesize the receiving locals. But a
// consumer of a generated .fly.h has no such sugar and builds the LOWERED form
// the archive symbol really exposes, supplying the out slots itself:
// `divmod(17, 5, q, r)` / `path.split(p, dir, base)`. Before B033 the lowered
// form matched because the synthetics were miscounted as explicit; stripping
// them fixed the sugar and broke the lowered form.
//
// They must NOT be tried together. Accepting both in one pass made an N-arg call
// match an (N-1)-explicit overload's lowered form just as readily as the N-arg
// overload meant for it — with two overloads of a value-returning function
// (`defaultOut(input)` / `defaultOut(input, ext)`) the lowered reading won, the
// second argument was bound as the out slot, and the compiler crashed in codegen.
// So: EXPLICIT is the real signature and always wins; LOWERED is a fallback tried
// only when no candidate matched explicitly (see the two passes in the lookups).
enum class ArityPass { Explicit, Lowered };

static bool MatchArity(SemaFunctionBase *Function, size_t NArgs, ArityPass Pass, size_t &Count) {
	size_t Explicit = ExplicitParamCount(Function);
	if (Pass == ArityPass::Explicit) {
		if (NArgs != Explicit) return false;
		Count = Explicit;
		return true;
	}
	if (NArgs == Function->getParams().size() && NArgs > Explicit) {
		Count = NArgs; // lowered form: the caller supplied the out slots
		return true;
	}
	return false;
}

Symbol *Registry::LookupFunction(llvm::StringRef Name, SmallVector<SemaType *, 8> &Types, SymbolTable *Scope) {
	if (Scope == nullptr) Scope = GlobalScope;
	llvm::SmallVector<Symbol *, 8> *Symbols = Scope->lookupInParents(Name);

	if (!Symbols) {
		return nullptr;
	}

	// Two passes over the candidates: every overload is offered its EXPLICIT
	// signature first, and only if none of them fits is the lowered form tried.
	// One combined pass let a lowered reading beat the overload actually meant
	// for the call (see MatchArity).
	for (ArityPass Pass : {ArityPass::Explicit, ArityPass::Lowered}) {
	// Iterate through all symbols with this name to find the right function
	for (Symbol *Sym : *Symbols) {

		// Check if symbol is a function
		if (Sym->getKind() != SymbolKind::FUNCTION) {
			continue;
		}

		SemaFunctionBase *Function = static_cast<SemaFunctionBase *>(Sym->getRef());
		llvm::SmallVector<SemaParam *, 8> &Params = Function->getParams();

		// Check if the argument count fits for THIS pass
		size_t MatchCount = 0;
		if (!MatchArity(Function, Types.size(), Pass, MatchCount)) {
			continue;
		}

		// Check if all parameter types match
		bool AllTypesMatch = true;
		for (size_t i = 0; i < MatchCount; i++) {
			SemaType *ParamType = Params[i]->getType();
			SemaType *ArgType = Types[i];

			// A null argument type is the `null`/unset literal: it matches any class param.
			if (!ArgType) {
				if (ParamType->isClass()) continue;
				AllTypesMatch = false;
				break;
			}

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

			// Allow class ↔ long: class pointers are pointer-sized (i64).
			if (ParamType->isNumber() && ParamType->isInteger() &&
			    static_cast<SemaIntType *>(ParamType)->getIntKind() == SemaIntTypeKind::TYPE_LONG &&
			    ArgType && ArgType->isClass()) {
				continue;
			}
			if (ParamType->isClass() && ArgType && ArgType->isNumber() && ArgType->isInteger() &&
			    static_cast<SemaIntType *>(ArgType)->getIntKind() == SemaIntTypeKind::TYPE_LONG) {
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
	} // end arity passes

	return nullptr;
}

// Exact signature lookup. Used to detect a DUPLICATE DECLARATION, so it compares
// EXPLICIT signatures only and never considers the lowered form: "the caller may
// supply the out slots" is a property of a call site, not of a declaration.
// Letting it through made `pick(a)` — whose params are (a, out) — collide with the
// signature of `pick(a, b)`, and the second overload of any value-returning
// function was rejected as "already defined in this scope". That is what broke the
// self-host's `defaultOut(input)` / `defaultOut(input, ext)` pair.
Symbol *Registry::LookupFunctionExact(llvm::StringRef Name, SmallVector<SemaType *, 8> &Types, SymbolTable *Scope) {
	if (Scope == nullptr) Scope = GlobalScope;
	llvm::SmallVector<Symbol *, 8> *Symbols = Scope->lookupInParents(Name);
	if (!Symbols) return nullptr;

	for (Symbol *Sym : *Symbols) {
		if (Sym->getKind() != SymbolKind::FUNCTION) continue;

		SemaFunctionBase *Function = static_cast<SemaFunctionBase *>(Sym->getRef());
		llvm::SmallVector<SemaParam *, 8> &Params = Function->getParams();

		size_t MatchCount = 0;
		if (!MatchArity(Function, Types.size(), ArityPass::Explicit, MatchCount)) continue;

		bool AllMatch = true;
		for (size_t i = 0; i < MatchCount; i++) {
			if (!Params[i]->getType()->isEquals(Types[i])) {
				AllMatch = false;
				break;
			}
		}
		if (AllMatch) return Sym;
	}
	return nullptr;
}

static bool FunctionTypesMatchExact(SemaFunctionBase *Function, SmallVector<SemaType *, 8> &Types, ArityPass Pass) {
	llvm::SmallVector<SemaParam *, 8> &Params = Function->getParams();
	size_t MatchCount = 0;
	if (!MatchArity(Function, Types.size(), Pass, MatchCount)) return false;
	for (size_t i = 0; i < MatchCount; i++) {
		SemaType *ParamType = Params[i]->getType();
		SemaType *ArgType = Types[i];
		// A null argument type is the `null`/unset literal: it matches any class param.
		if (!ArgType) { if (ParamType->isClass()) continue; return false; }
		if (ParamType->isEquals(ArgType)) continue;
		if (ParamType->isClass() && ArgType->isClass()) {
			if (static_cast<SemaClassType *>(ArgType)->isDerivedOrEquals(
			        static_cast<SemaClassType *>(ParamType))) continue;
		}
		// class ↔ long coercion is intentionally excluded here so that an
		// exact class type match (fly.mem.Ptr) wins over a long overload.
		// The coercion is handled in the second-pass FunctionTypesMatch below.
		return false;
	}
	return true;
}

// Strict identity match: class-typed params must be the SAME class (isEquals),
// never a derived→base upcast. Used to break ties when the (looser) exact pass
// yields several candidates that differ only by inheritance distance — the most
// derived / identical overload should win (C++ overload-ranking semantics).
static bool FunctionTypesMatchStrict(SemaFunctionBase *Function, SmallVector<SemaType *, 8> &Types, ArityPass Pass) {
	llvm::SmallVector<SemaParam *, 8> &Params = Function->getParams();
	size_t MatchCount = 0;
	if (!MatchArity(Function, Types.size(), Pass, MatchCount)) return false;
	for (size_t i = 0; i < MatchCount; i++) {
		SemaType *ParamType = Params[i]->getType();
		SemaType *ArgType = Types[i];
		if (!ArgType) { if (ParamType->isClass()) continue; return false; }
		if (!ParamType->isEquals(ArgType)) return false;
	}
	return true;
}

static bool FunctionTypesMatch(SemaFunctionBase *Function, SmallVector<SemaType *, 8> &Types, ArityPass Pass) {
	llvm::SmallVector<SemaParam *, 8> &Params = Function->getParams();
	size_t MatchCount = 0;
	if (!MatchArity(Function, Types.size(), Pass, MatchCount)) return false;
	for (size_t i = 0; i < MatchCount; i++) {
		SemaType *ParamType = Params[i]->getType();
		SemaType *ArgType = Types[i];
		// A null argument type is the `null`/unset literal: it matches any class param.
		if (!ArgType) { if (ParamType->isClass()) continue; return false; }
		if (ParamType->isEquals(ArgType)) continue;
		if (ParamType->isClass() && ArgType->isClass()) {
			if (static_cast<SemaClassType *>(ArgType)->isDerivedOrEquals(
			        static_cast<SemaClassType *>(ParamType))) continue;
		}
		if (ParamType->isNumber() && ArgType->isNumber()) continue;
		// Allow class ↔ long: class pointers are pointer-sized (i64).
		if (ParamType->isNumber() && ParamType->isInteger() &&
		    static_cast<SemaIntType *>(ParamType)->getIntKind() == SemaIntTypeKind::TYPE_LONG &&
		    ArgType && ArgType->isClass()) continue;
		if (ParamType->isClass() && ArgType && ArgType->isNumber() && ArgType->isInteger() &&
		    static_cast<SemaIntType *>(ArgType)->getIntKind() == SemaIntTypeKind::TYPE_LONG) continue;
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

	// The whole ranking below runs against the EXPLICIT signatures first; only if
	// that yields no candidate at all is it repeated allowing the lowered form.
	// Ranking the two arities together let a lowered reading of one overload
	// outrank the overload actually written for the call.
	for (ArityPass Pass : {ArityPass::Explicit, ArityPass::Lowered}) {

	// First pass: prefer exact matches (no numeric promotion) to avoid ambiguity
	// when multiple overloads differ only in numeric type (e.g. int vs long).
	llvm::SmallVector<Symbol *, 4> ExactResult;
	for (Symbol *Sym : *Symbols) {
		if (Sym->getKind() != SymbolKind::FUNCTION) continue;
		if (FunctionTypesMatchExact(static_cast<SemaFunctionBase *>(Sym->getRef()), Types, Pass))
			ExactResult.push_back(Sym);
	}
	// Tie-break: if several candidates passed the (inheritance-tolerant) exact
	// pass, prefer a uniquely identical (no-upcast) overload so the most derived
	// match wins instead of producing a spurious ambiguity.
	if (ExactResult.size() > 1) {
		llvm::SmallVector<Symbol *, 4> StrictResult;
		for (Symbol *Sym : ExactResult) {
			if (FunctionTypesMatchStrict(static_cast<SemaFunctionBase *>(Sym->getRef()), Types, Pass))
				StrictResult.push_back(Sym);
		}
		if (StrictResult.size() == 1) return StrictResult;
	}
	if (!ExactResult.empty()) return ExactResult;

	// Second pass: fall back to promotion-aware matches.
	llvm::SmallVector<Symbol *, 4> Result;
	for (Symbol *Sym : *Symbols) {
		if (Sym->getKind() != SymbolKind::FUNCTION) continue;
		if (FunctionTypesMatch(static_cast<SemaFunctionBase *>(Sym->getRef()), Types, Pass))
			Result.push_back(Sym);
	}
	if (!Result.empty()) return Result;

	} // end arity passes

	return {};
}
