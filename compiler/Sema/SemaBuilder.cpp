//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaBuilder.cpp - semantic analysis builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuilder.h"

#include "AST/ASTBinary.h"
#include "AST/ASTCall.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTFunction.h"
#include "AST/ASTMember.h"
#include "AST/ASTTernary.h"
#include "AST/ASTUnary.h"
#include "AST/ASTVar.h"
#include "Basic/Debug.h"
#include "Basic/Diagnostic.h"
#include "Sema/SemaBinary.h"
#include "Sema/SemaCall.h"
#include "Sema/SemaCast.h"
#include "Sema/SemaClassAttribute.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaComment.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaBlockStmt.h"
#include "Sema/SemaDeclStmt.h"
#include "Sema/SemaExprStmt.h"
#include "Sema/SemaReturnStmt.h"
#include "Sema/SemaIfStmt.h"
#include "Sema/SemaSwitchStmt.h"
#include "Sema/SemaLoopStmt.h"
#include "Sema/SemaLoopInStmt.h"
#include "Sema/SemaDeleteStmt.h"
#include "Sema/SemaBreakStmt.h"
#include "Sema/SemaContinueStmt.h"
#include "Sema/SemaFailStmt.h"
#include "Sema/SemaHandleStmt.h"
#include "Sema/SemaEnumEntry.h"
#include "Sema/SemaEnumList.h"
#include "Sema/SemaEnumAccessor.h"
#include "Sema/SemaError.h"
#include "Sema/SemaFunction.h"
#include "Sema/SemaMember.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaTernary.h"
#include "Sema/SemaType.h"
#include "Sema/SemaUnary.h"
#include "Sema/SemaValue.h"
#include "Sema/SymbolTable.h"

#include "llvm/Support/Regex.h"

#include <AST/ASTAttribute.h>
#include <AST/ASTBuilder.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTMethod.h>
#include <AST/ASTParam.h>
#include <AST/ASTValue.h>
#include <Sema/SemaBuilderModifiers.h>
#include <Sema/SemaBuiltin.h>
#include <Sema/SemaClassInstance.h>
#include <Sema/SemaLocalVar.h>
#include <Sema/SemaNameSpace.h>
#include <Sema/SemaParam.h>

#include <string>

using namespace fly;

// --- Generic specialization helpers ---

static std::string mangleSemaType(SemaType *T) {
	if (T->isInteger()) {
		switch (static_cast<SemaIntType *>(T)->getIntKind()) {
			case SemaIntTypeKind::TYPE_BYTE:   return "Y";
			case SemaIntTypeKind::TYPE_SHORT:  return "S";
			case SemaIntTypeKind::TYPE_INT:    return "I";
			case SemaIntTypeKind::TYPE_LONG:   return "L";
			case SemaIntTypeKind::TYPE_USHORT: return "Us";
			case SemaIntTypeKind::TYPE_UINT:   return "Ui";
			case SemaIntTypeKind::TYPE_ULONG:  return "U";
			case SemaIntTypeKind::TYPE_POINTER: return "Pz";
		}
	}
	if (T->isFloat()) {
		switch (static_cast<SemaFloatType *>(T)->getFloatKind()) {
			case SemaFloatTypeKind::TYPE_FLOAT:  return "F";
			case SemaFloatTypeKind::TYPE_DOUBLE: return "D";
		}
	}
	if (T->isBool())   return "B";
	if (T->isString()) return "Ss";
	if (T->isClass()) {
		std::string N = T->getName();
		return "C" + std::to_string(N.size()) + N;
	}
	return "X";
}

static std::string buildMangleKey(const std::string &BaseName,
                                  const llvm::SmallVector<SemaType *, 4> &TypeArgs) {
	std::string Key = BaseName + "_";
	for (auto *T : TypeArgs) Key += mangleSemaType(T);
	return Key;
}

SemaImport *SemaBuilder::CreateImport(SemaModule &Module, ASTImport &AST) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateImport");

	// Search Namespace in Symbol Table
	SemaImport *Import = new SemaImport(AST);

	// Add Import to the Module Imports for next symbols resolution
	Module.addImport(Import);
	return Import;
}

SemaFunction *SemaBuilder::CreateFunction(SemaModule &Module, SymbolTable *Symbols, ASTFunction &AST) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateFunction");

	// Create the Function
	SemaFunction *Function = new SemaFunction(AST, Symbols);

	SemaBuilderModifiers *BuilderModifiers = SemaBuilderModifiers::Build(AST.getModifiers());
	Function->setVisibility(BuilderModifiers->getVisibility());

	// Add to Module
	Module.addNode(Function);
	return Function;
}

SemaClassType * SemaBuilder::CreateClass(SemaModule &Module, SymbolTable *Symbols, ASTClass &AST) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateClass");

	// Create the Class Type
	SemaClassType *Class = new SemaClassType(AST, Module, Symbols);

	// Set Symbol Table
	Class->Symbols = Symbols;

	// Create the 'this' attribute for the current class
	Class->This = new SemaClassInstance(Class);

	// Set Modifiers
	SemaBuilderModifiers *BuilderModifiers = SemaBuilderModifiers::Build(AST.getModifiers());
	Class->Constant = BuilderModifiers->isConstant();
	Class->Visibility = BuilderModifiers->getVisibility();
	Class->Abstract = BuilderModifiers->isAbstract();
	Class->Final = BuilderModifiers->isFinal();

	// Add to Module
	Module.addNode(Class);
	return Class;
}

SemaClassType *SemaBuilder::CreateSpecialization(SemaClassType *Template,
                                                  llvm::SmallVector<SemaType *, 4> TypeArgs,
                                                  SymbolTable *Symbols) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateSpecialization");

	// Build mangle key and check cache
	std::string Key = buildMangleKey(std::string(Template->getName()), TypeArgs);
	auto It = Template->Specializations.find(Key);
	if (It != Template->Specializations.end())
		return It->second;

	// Create a fresh scope whose parent is the template scope's parent (module scope),
	// so that module-level type lookups work but type params resolve to concrete types.
	SymbolTable *SpecScope = new SymbolTable(Template->getSymbols()->getParent());

	// Register each type arg under the type param's name (e.g. T → int)
	const auto &TPs = Template->getTypeParams();
	for (size_t i = 0; i < TPs.size() && i < TypeArgs.size(); ++i) {
		Symbol *ParamSym = new Symbol(std::string(TPs[i]->getName()), SymbolKind::CLASS, TypeArgs[i]);
		SpecScope->insert(ParamSym);
	}

	// Build the specialization shell (reuses template AST and module)
	SemaClassType *Spec = new SemaClassType(Template->getAST(), Template->getModule(), SpecScope);
	Spec->MangledName   = Key;
	Spec->GenericTemplate = Template;
	Spec->Visibility    = Template->Visibility;
	Spec->Abstract      = Template->Abstract;
	Spec->Final         = Template->Final;
	Spec->Constant      = Template->Constant;
	Spec->ClassKind     = Template->ClassKind;
	Spec->This          = new SemaClassInstance(Spec);

	// Cache before returning (prevents infinite recursion if T appears in its own body)
	Template->Specializations[Key] = Spec;

	return Spec;
}

SemaFunction *SemaBuilder::CreateFunctionSpecialization(
    SemaFunction *Template,
    llvm::SmallVector<SemaType *, 4> TypeArgs,
    SymbolTable *Symbols) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateFunctionSpecialization");

	// Build mangle key and check cache
	std::string Key = buildMangleKey(std::string(Template->getAST().getName()), TypeArgs);
	auto It = Template->Specializations.find(Key);
	if (It != Template->Specializations.end())
		return It->second;

	// Create spec scope with T→concrete type bindings; parent = template scope's parent (module scope)
	SymbolTable *SpecScope = new SymbolTable(Template->getSymbols()->getParent());

	const auto &TPs = Template->getTypeParams();
	for (size_t i = 0; i < TPs.size() && i < TypeArgs.size(); ++i) {
		Symbol *ParamSym = new Symbol(std::string(TPs[i]->getName()), SymbolKind::CLASS, TypeArgs[i]);
		SpecScope->insert(ParamSym);
	}

	// Build the specialization (reuses the template's ASTFunction)
	SemaFunction *Spec = new SemaFunction(Template->getAST(), SpecScope);
	Spec->MangledName = Key;
	Spec->GenericTemplate = Template;
	Spec->setVisibility(Template->getVisibility());
	Spec->setNamespaceName(Template->getNamespaceName());

	// Cache before returning
	Template->Specializations[Key] = Spec;

	return Spec;
}

SemaClassAttribute * SemaBuilder::CreateClassAttribute(SemaClassType &Class, ASTAttribute &AST, SemaType *Type) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateClassAttribute");

	SemaClassAttribute *Attribute = new SemaClassAttribute(AST, Class, Type);
	Attribute->setParent(*Class.getThis());

	// Set Modifiers
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST.getModifiers());
	Attribute->Visibility = Builder->getVisibility();
	Attribute->Static = Builder->isStatic();
	Attribute->Constant = Builder->isConstant();
	return Attribute;
}

SemaClassMethod * SemaBuilder::CreateDefaultConstructor(SemaClassType *Class, SymbolTable* Scope) {
	// Create AST
	ASTMethod *AST = ASTBuilder::CreateDefaultConstructor(&Class->getAST());

	// Create Sema
	SemaClassMethod *Method = new SemaClassMethod(*AST, Class, Class->getThis(), true, Scope);

	return Method;
}

SemaClassMethod * SemaBuilder::CreateClassMethod(SemaClassType *Class, ASTMethod &AST, SymbolTable* Scope) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateClassFunction");
	SemaClassMethod *Method;

	// Set Modifiers first so we can use them for method kind determination
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST.getModifiers());

	bool IsConstructor = (AST.getName() == Class->getName());
	Method = new SemaClassMethod(AST, Class, Class->getThis(), IsConstructor, Scope);

	Method->Visibility = Builder->getVisibility();
	Method->Static = Builder->isStatic();
	Method->Final = Builder->isFinal();
	return Method;
}

SemaEnumType * SemaBuilder::CreateEnum(SemaModule &Module, SymbolTable *Symbols, ASTEnum &AST) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateEnum");

	SemaEnumType *Enum = new SemaEnumType(AST, Symbols);

	// Set Symbol Table
	Enum->Symbols = Symbols;

	// Set Modifiers
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST.getModifiers());
	Enum->Visibility = Builder->getVisibility();
	Enum->Constant = Builder->isConstant();
	Enum->Comment = nullptr;

	// Add to Module
	Module.addNode(Enum);
	return Enum;
}

SemaEnumEntry * SemaBuilder::CreateEnumEntry(SemaEnumType *Enum, ASTEnumEntry &AST) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateEnumEntry");

	SemaEnumEntry *Entry = new SemaEnumEntry(AST, Enum);
	// Start index from 1, so 0 can be used as undefined/default value
	Entry->Index = Enum->Entries.size() + 1;
	Enum->Entries.insert(std::make_pair(AST.getName(), Entry));
	return Entry;
}

SemaEnumList * SemaBuilder::CreateEnumList(SemaEnumType *EnumType) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateEnumList");

	// Create an array type of the enum type with size = number of entries
	uint64_t Size = EnumType->getEntries().size();
	SemaArrayType *ArrayType = SemaBuiltin::CreateArrayType(EnumType, Size);

	SemaEnumList *EnumList = new SemaEnumList(EnumType, ArrayType);
	return EnumList;
}

SemaEnumAccessor * SemaBuilder::CreateEnumAccessor(SemaEnumType *EnumType, SemaEnumEntry *Entry,
                                                   SemaVar *Var, bool IsName) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateEnumAccessor");

	// .name yields a string, .value yields an int.
	SemaType *Type = IsName ? SemaBuiltin::getStringType()
	                        : static_cast<SemaType *>(SemaBuiltin::getIntType());

	return new SemaEnumAccessor(EnumType, Entry, Var, IsName, Type);
}

SemaComment * SemaBuilder::CreateComment(ASTComment &AST) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateComment");

	SemaComment * Comment = new SemaComment(AST);
	return Comment;
}

SemaLocalVar * SemaBuilder::CreateLocalVar(ASTLocalVar &AST, SemaType *Type) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateLocalVar");

	// Create LocalVar Symbol
	SemaLocalVar *Sema = new SemaLocalVar(AST, Type);
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST.getModifiers());
	Sema->Constant = Builder->isConstant();
	return Sema;
}

SemaParam *SemaBuilder::CreateParam(ASTParam &AST, SemaType *Type) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateParam");

	// Create LocalVar Symbol
	SemaParam *Sema = new SemaParam(AST, Type);

	// Set Constant from modifiers
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST.getModifiers());
	Sema->Constant = Builder->isConstant();
	return Sema;
}

SemaMember * SemaBuilder::CreateMemberVar(ASTMember &AST, SemaExpr *Ref, SemaExpr *Parent) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateMemberVar");

	SemaMember *Sema = new SemaMember(AST, Ref, Parent);
	return Sema;
}

SemaError *SemaBuilder::CreateErrorHandler() {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateErrorHandler");

	SemaError * Sema = new SemaError(nullptr);
	return Sema;
}

SemaUnary *SemaBuilder::CreateUnary(ASTUnary &AST, SemaExpr *Expr) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateUnary");

	// Create Unary Symbol
	SemaUnary *Sema = new SemaUnary(AST, Expr);
	return Sema;
}

SemaBinary *SemaBuilder::CreateBinary(ASTBinary &AST, SemaExpr *Left, SemaExpr *Right) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateBinary");

	// Create Binary Symbol
	SemaBinary *Sema = new SemaBinary(AST, Left, Right);
	return Sema;
}

SemaTernary *SemaBuilder::CreateTernary(ASTTernary &AST, SemaExpr *Cond, SemaExpr *TrueExpr, SemaExpr *FalseExpr) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateTernary");

	// Create Ternary Symbol
	SemaTernary *Sema = new SemaTernary(AST, Cond, TrueExpr, FalseExpr);
	return Sema;
}

SemaCast *SemaBuilder::CreateCast(ASTCast &AST, SemaExpr *Expr, SemaType *ToType) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateCast");

	SemaCast *Sema = new SemaCast(AST, Expr, ToType);
	return Sema;
}

SemaCall * SemaBuilder::CreateCall(ASTCall &AST, SemaType *Type, SemaFunctionBase *Function) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateParam");

	// Create Call Symbol
	SemaCall *Call = new SemaCall(AST, Type);
	Call->Function = Function;
	return Call;
}

SemaBoolValue * SemaBuilder::CreateBoolValue(ASTBoolValue &AST) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateBoolValue");

	SemaBoolValue * V = new SemaBoolValue(AST);
	return V;
}

SemaValue * SemaBuilder::CreateNumberValue(ASTNumberValue &AST) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateNumberValue");

	SemaValue *Sema;

	llvm::StringRef Raw = AST.getValue();

	// Strip digit separators
	std::string Stripped;
	Stripped.reserve(Raw.size());
	for (char C : Raw) {
		if (C != '_')
			Stripped += C;
	}
	llvm::StringRef ValStr(Stripped);

	// Imaginary suffix: ends with 'j' or 'J' → complex value with real=0
	if (ValStr.ends_with("j") || ValStr.ends_with("J")) {
		llvm::StringRef ImagStr = ValStr.drop_back(1);
		llvm::APFloat Imag(llvm::APFloat::IEEEdouble(), ImagStr);
		llvm::APFloat Real = llvm::APFloat::getZero(llvm::APFloat::IEEEdouble());
		Sema = new SemaComplexValue(AST, SemaBuiltin::getComplexType(), Real, Imag);
		return Sema;
	}

	// Floating point number (contains '.' or 'e'/'E' exponent)
	llvm::Regex FloatRegex(R"(^[-+]?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)?$)");
	if (FloatRegex.match(ValStr)) {
		// Floating point
		llvm::APFloat Value = llvm::APFloat(llvm::APFloat::IEEEdouble(), ValStr);
		Sema = new SemaFloatValue(AST, SemaBuiltin::getFloatType(), Value);
	} else {
		llvm::APInt Value = CreateAPIntValue(ValStr);

		// Compute MinBits for type inference.
		// For the negative branch use countLeadingOnes (two's complement sign bits).
		// For the positive branch use getActiveBits() so the bit-width of the
		// (large) parse buffer does not inflate the count.
		unsigned MinBits = Value.isNegative()
			? 1 + Value.getBitWidth() - Value.countLeadingOnes()
			: Value.getActiveBits() + 1;

		// Infer Type based on MinBits, but keep the Value at its original bit width.
		// Positive decimal literals follow signed-type convention (same as C/Rust):
		// a value that fits in int is typed as int, not uint.
		SemaIntType *Type = nullptr;
		if (Value.isNegative()) {
			if (MinBits <= 16) Type = SemaBuiltin::getShortType();
			else if (MinBits <= 32) Type = SemaBuiltin::getIntType();
			else Type = SemaBuiltin::getLongType();
		} else {
			if (MinBits <= 8)       Type = SemaBuiltin::getByteType();
			else if (MinBits <= 16) Type = SemaBuiltin::getShortType();
			else if (MinBits <= 32) Type = SemaBuiltin::getIntType();
			else if (MinBits <= 64) Type = SemaBuiltin::getLongType();
			else                    Type = SemaBuiltin::getULongType();
		}

		// Create SemaIntValue with the full-width Value (truncation will happen during codegen)
		Sema = new SemaIntValue(AST, Type, Value);
	}
	return Sema;
}

llvm::APInt SemaBuilder::CreateAPIntValue(StringRef ValStr) {
	// Strip digit separators
	std::string Stripped;
	Stripped.reserve(ValStr.size());
	for (char C : ValStr) {
		if (C != '_')
			Stripped += C;
	}
	ValStr = StringRef(Stripped);

	bool IsNegative = ValStr.starts_with("-");
	if (IsNegative)
		ValStr = ValStr.drop_front(1);

	// Detect radix
	unsigned Radix = 10;
	if (ValStr.starts_with("0x") || ValStr.starts_with("0X")) {
		Radix = 16;
		ValStr = ValStr.drop_front(2);
	} else if (ValStr.starts_with("0b") || ValStr.starts_with("0B")) {
		Radix = 2;
		ValStr = ValStr.drop_front(2);
	} else if (ValStr.starts_with("0o") || ValStr.starts_with("0O")) {
		Radix = 8;
		ValStr = ValStr.drop_front(2);
	}

	// Parse with sufficient precision (128-bit) so that values like INT_MAX
	// (2147483647 = 0x7FFFFFFF) don't lose their high bit due to getBitsNeeded
	// returning a width that is too small for unsigned representation.
	unsigned SrcBits = std::max(128u, llvm::APInt::getBitsNeeded(ValStr, Radix));
	llvm::APInt Value(SrcBits, ValStr, Radix);
	return IsNegative ? -Value : Value;
}

SemaIntValue * SemaBuilder::CreateIntValue(ASTNumberValue &AST, SemaIntType *IntType) {
	llvm::APInt Value = CreateAPIntValue(AST.getValue());
	SemaIntValue *Sema = new SemaIntValue(AST, IntType, Value);
	return Sema;
}

SemaFloatValue * SemaBuilder::CreateFloatValue(ASTNumberValue &AST, SemaFloatType *FloatType) {
	std::string Stripped;
	for (char C : AST.getValue())
		if (C != '_') Stripped += C;
	llvm::APFloat Value = llvm::APFloat(llvm::APFloat::IEEEdouble(), Stripped);
	SemaFloatValue *Sema = new SemaFloatValue(AST, FloatType, Value);
	return Sema;
}

SemaStringValue * SemaBuilder::CreateStringValue(ASTStringValue &AST) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateStringValue");

	SemaStringValue * V = new SemaStringValue(AST);
	V->Value = AST.getValue();
	V->Type = SemaBuiltin::getStringType();
	return V;
}

SemaArrayValue * SemaBuilder::CreateArrayValue(ASTArrayValue &AST, SemaType *Type, llvm::SmallVector<SemaValue *, 8> &Values) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateArrayValue");

	uint64_t Size = Values.size();
	SemaArrayType *ArrayType = SemaBuiltin::CreateArrayType(Type, Size);
	SemaArrayValue * V = new SemaArrayValue(AST, ArrayType);
	V->Values = std::move(Values);
	return V;
}

SemaStructValue * SemaBuilder::CreateStructValue(ASTStructValue &AST, llvm::StringMap<SemaValue *> Values) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateStructValue");

	llvm::SmallVector<SemaType *, 8> Types;
	for (auto &Entry : Values) {
		Types.push_back(Entry.second->getType());
	}

	SemaStructValue * V = new SemaStructValue(AST, nullptr);
	V->Values = std::move(Values);
	return V;
}

SemaValue * SemaBuilder:: CreateNullValue(ASTNullValue &AST) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateNullValue");

	SemaValue * V = new SemaNullValue(AST);
	return V;
}

SemaValue *SemaBuilder::CreateUnsetValue(ASTUnsetValue &AST) {
	FLY_DEBUG_SCOPE("SemaBuilder", "CreateUnsetValue");
	SemaValue * V = new SemaUnsetValue(AST);
	return V;
}

// ─── SemaStmt factory methods ───────────────────────────────────────────────

SemaBlockStmt *SemaBuilder::CreateBlockStmt(ASTStmt *AST) {
	return new SemaBlockStmt(AST);
}

SemaDeclStmt *SemaBuilder::CreateDeclStmt(ASTStmt *AST, SemaLocalVar *Var, SemaExpr *Expr) {
	return new SemaDeclStmt(AST, Var, Expr);
}

SemaExprStmt *SemaBuilder::CreateExprStmt(ASTStmt *AST, SemaExpr *Expr) {
	return new SemaExprStmt(AST, Expr);
}

SemaReturnStmt *SemaBuilder::CreateReturnStmt(ASTStmt *AST) {
	return new SemaReturnStmt(AST);
}

SemaIfStmt *SemaBuilder::CreateIfStmt(ASTStmt *AST, SemaExpr *Cond, SemaStmt *Then) {
	return new SemaIfStmt(AST, Cond, Then);
}

SemaSwitchStmt *SemaBuilder::CreateSwitchStmt(ASTStmt *AST, SemaExpr *Expr) {
	return new SemaSwitchStmt(AST, Expr);
}

SemaLoopStmt *SemaBuilder::CreateLoopStmt(ASTStmt *AST, bool VerifyAtEnd) {
	return new SemaLoopStmt(AST, VerifyAtEnd);
}

SemaLoopInStmt *SemaBuilder::CreateLoopInStmt(ASTStmt *AST, SemaExpr *Item, SemaExpr *List, SemaStmt *Body) {
	return new SemaLoopInStmt(AST, Item, List, Body);
}

SemaDeleteStmt *SemaBuilder::CreateDeleteStmt(ASTStmt *AST, SemaExpr *Expr) {
	return new SemaDeleteStmt(AST, Expr);
}

SemaBreakStmt *SemaBuilder::CreateBreakStmt(ASTStmt *AST) {
	return new SemaBreakStmt(AST);
}

SemaContinueStmt *SemaBuilder::CreateContinueStmt(ASTStmt *AST) {
	return new SemaContinueStmt(AST);
}

SemaFailStmt *SemaBuilder::CreateFailStmt(ASTStmt *AST) {
	return new SemaFailStmt(AST);
}

SemaHandleStmt *SemaBuilder::CreateHandleStmt(ASTStmt *AST) {
	return new SemaHandleStmt(AST);
}
