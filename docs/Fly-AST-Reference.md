# Fly AST & Sema Reference

This document describes every Abstract Syntax Tree (AST) construct produced by the Fly parser and how those nodes are translated into Semantic Analysis (Sema) objects before the CodeGen pass. It concentrates on symbol table formation and on the `SemaType`, `SemaVar`, `SemaCall`, and `SemaMember*` families that participate in symbol resolution.

## 1. Pipeline Overview
- **Parsing** builds tree nodes derived from `ASTBase`/`ASTNode`. Nodes retain source locations and syntactic kinds only.
- **SemaBuilder** allocates Sema nodes (subclasses of `SemaNode`) mirroring the AST but enriched with types, binding, and scope ownership.
- **Resolver** walks the AST via `ASTVisitor`, interleaving symbol table insertion/lookup (`SymbolTable`, `Symbol`). It wires AST references (identifiers, members, calls) to the appropriate Sema objects and finalizes types.
- **SemaValidator** runs after resolution to ensure invariants before lowering into CodeGen.

Only after these stages does CodeGen consult the Sema graph (`CodeGen*` pointers on Sema classes are initially `nullptr`).

## 2. AST Fundamentals
| Component | Location | Notes |
|-----------|----------|-------|
| `ASTBase` | `include/AST/ASTBase.h` | Stores `SourceLocation` and `ASTKind`. Provides `str()` helpers and list formatting utilities used by diagnostics.
| `ASTNode` | `include/AST/ASTNode.h` | Abstract base for visitable syntax nodes. Carries a `Visited` bit and pure `accept(ASTVisitor&)`.
| `ASTVisitor` | `include/AST/ASTVisitor.h` | Double-dispatch visitor implemented by the resolver.
| `ASTKind` | `include/AST/ASTBase.h` | Enumerates top-level syntactic categories: module, namespace, import, var, stmt, type, expr, etc.

All AST nodes own child pointers directly (no arena). Most nodes expose getters/setters but never perform semantic analysis themselves.

## 3. AST Inventory
### 3.1 Modules, Namespaces, Imports
- `ASTModule` (`include/AST/ASTModule.h`): Root per translation unit, records file, namespace, and ordered top-level nodes.
- `ASTNameSpace`: Represents the namespace declaration header. A module may omit this, in which case the resolver uses the registry’s default namespace.
- `ASTImport`: Stores a qualified name (`ASTName` segments) plus an optional alias.

### 3.2 Types
- `ASTType` + `ASTTypeKind`: abstract identity types (`TYPE_NAMED`, `TYPE_BUILTIN`, `TYPE_ARRAY`). Each holds an eventual `SemaType*` once resolved.
- `ASTBuiltinType`: enumerates keywords such as `int`, `double`, `string`, `void`, `error`.
- `ASTNamedType`: a chain of names referencing namespace-qualified types.
- `ASTArrayType`: wraps an element type and an optional size expression.

### 3.3 Values & Literals
`ASTValue` specializes into `ASTBoolValue`, `ASTNumberValue`, `ASTStringValue`, `ASTArrayValue`, `ASTStructValue`, `ASTNullValue`, and `ASTDefaultValue`. Each literal stores raw textual payload and later maps to `SemaValue` subclasses.

### 3.4 Variables & Parameters
- `ASTVar`: common base with `ASTVarKind` (local, param, attribute, enum entry). Holds declared type, modifiers, initializer expression, and future `SemaVar*`.
- Subclasses: `ASTLocalVar`, `ASTParam`, `ASTAttribute`, `ASTEnumEntry`.

### 3.5 Expressions
`ASTExpr` defines the expression tree skeleton with parent/child links and `SemaExpr*`/`SemaType*` slots. Concrete forms:
- Identifiers (`ASTIdentifier`), members (`ASTMember`), calls (`ASTCall`).
- Operators (`ASTUnaryOp`, `ASTBinaryOp`, `ASTTernaryOp`).
- Casts (`ASTCast`).
- Literal expressions via `ASTValue` derivatives.

### 3.6 Statements & Blocks
`ASTStmt` and `ASTStmtKind` cover control-flow and simple statements: blocks, expression statements, assignments, delete, fail, handle, return, rule, break/continue, switch, loop, loop-in. `ASTBlockStmt` provides statement sequencing plus a `StringMap` of in-scope locals.

### 3.7 Functions, Methods, Classes, Enums
- `ASTFunction` / `ASTFunctionKind`: functions capture signature, modifiers, body, and comments. Methods (`ASTMethod`) inherit and represent class-scoped functions.
- `ASTClass`: aggregates modifiers, name, bases, and nested nodes (attributes, methods, constructors).
- `ASTEnum`: similar container for enum entries and optional bases.

## 4. Sema Fundamentals
| Component | Location | Notes |
|-----------|----------|-------|
| `SemaNode` | `include/Sema/SemaNode.h` | Base class with `SemaKind` enumeration (module, namespace, import, type, var, call, op, function, class, attribute, method, enum, enum entry, value).
| `SemaExpr` | `include/Sema/SemaExpr.h` | Adds parent/child expression linkage and cached `SemaType*`.
| `SemaFunctionBase` | `include/Sema/SemaFunctionBase.h` | Common state for free functions and class methods: mangled name, params, locals, return type, error handler, `CodeGenFunctionBase*` placeholder.
| `SemaBuilder` | `include/Sema/SemaBuilder.h` | Factory for every Sema subtype (functions, classes, vars, literals, calls, etc.).
| `Resolver` | `include/Sema/Resolver.h`, `src/Sema/Resolver.cpp` | Main visitor bridging AST to Sema. Manages scopes, modules, name spaces, classes/enums, and expressions.
| `Registry` | `include/Sema/Registry.h` | Tracks modules, namespaces, and built-in scopes used by the resolver.

## 5. Sema Types (Detailed)
`SemaType` encapsulates semantic type identity and default value behavior.
- **Core fields**: immutable `Id`, `SemaTypeKind`, `Name`, optional `SemaValue* DefaultValue`. Predicates (`isBool`, `isInteger`, `isArray`, etc.) drive type checking.
- **Equality**: pointer equality wrappers plus `isEquals` for structural checks (arrays compare element types/size expressions).

### Derived Types
| Class | Notes |
|-------|-------|
| `SemaIntType` | Wraps `SemaIntTypeKind` (byte/ushort/short/uint/int/ulong/long). Signedness is encoded by the enum value (low bit indicates signed).
| `SemaFloatType` | Wraps `SemaFloatTypeKind` (`float`, `double`).
| `SemaArrayType` | References the element `SemaType*` and AST size expression for deferred constant folding.
| `SemaClassType` | Rich type representing class/interface/struct definitions. Holds module/namespace ownership, symbol table, nodes (attributes, methods, ctors), visibility, constant flag, base classes, `this` instance, maps of attributes/methods/ctors, and optional comment/CodeGen objects.
| `SemaEnumType` | Similar management for enums: symbol table, base enums, visibility, constant flag, entry map.
| `SemaErrorType` | Sentinel type returned when resolution fails to prevent cascading issues.

### Builtins
`SemaBuiltin` exposes singleton getters for primitive types and creates array types when needed. Resolver consults `Registry::LookupBuiltinType` before searching namespaces.

## 6. Symbol Table & Scope Model
- `SymbolTable` (`include/Sema/SymbolTable.h`) implements a scoped lookup chain backed by `llvm::StringMap<Symbol*>`. `pushScope()` creates a child table linked via `Parent`.
- `Symbol` (`include/Sema/Symbol.h`) bundles a `Name`, `SemaKind`, and pointer to the referenced `SemaNode`.
- Resolver maintains `CurrentScope` and manipulates scopes via `EnterScope`/`ExitScope`. Local scopes correspond to blocks/functions/classes; module and namespace scopes come from the registry or class/enum symbol tables.
- Symbols are inserted immediately after SemaBuilder creates the corresponding semantic object (e.g., a function symbol inserted in the parent scope before resolving the body).

## 7. Sema Variables (Detailed)
`SemaVar` (`include/Sema/SemaVar.h`) is the semantic counterpart for any named storage.
- **Core fields**: owning `ASTVar*`, `SemaVarKind` (param, local, member, error, class attribute, class instance, enum entry), `Constant` flag (derived from modifiers), `CodeGenVarBase*` placeholder.
- **Naming**: `getName()` defers to the AST by default; some subclasses override for synthetic identifiers (`SemaClassInstance` uses "this").

### Subclasses
| Class | Purpose |
|-------|---------|
| `SemaLocalVar` | Function/block locals. Owns `CodeGenVar*` placeholder.
| `SemaParam` | Function parameter variables; retains order and codegen slot.
| `SemaMemberVar` | Result of resolving an `ASTMember` expression. Stores pointer to the underlying `SemaClassAttribute*`, (optional) `CodeGenVar*`, and inherits constant flag from attribute definition.
| `SemaClassAttribute` | Declaration-time representation of class/struct fields. Records parent `SemaClassType`, visibility, static flag, inherited attribute reference, comment, and codegen slot.
| `SemaClassInstance` | Synthetic `this` variable for methods. Maintains a map from base type id → base instance to support `super` style access.
| `SemaEnumEntry` | Enum constants, each with index, comment, and `CodeGenEnumEntry*` placeholder.

The resolver inserts matching `Symbol`s for each declaration so that future identifier/member lookups retrieve the `SemaVar` instead of raw AST nodes.

## 8. Member Access Resolution
1. Parser produces `ASTMember` nodes (holding name, parent expression pointer, `ASTVar*` for the definition).
2. Resolver evaluates the parent expression to obtain a `SemaExpr*`. If the parent is a namespace, class, enum, or call, it delegates to the correct `ResolveChild` overload.
3. `ResolveChildMember` locates the target attribute via `SemaClassType::LookupAttribute`. It instantiates a `SemaMemberVar` via `SemaBuilder::CreateMemberVar`, linking the AST member, parent expression, and referenced `SemaClassAttribute`.
4. The resulting `SemaMemberVar` inherits the attribute’s type, visibility, constant flag, and inserted symbol table entry (typically within the class scope when the attribute is declared, and within the current expression scope for temporary references).

## 9. Calls & Function Binding
`SemaCall` (`include/Sema/SemaCall.h`) represents invocation expressions and stores:
- Reference to the originating `ASTCall` (`getAST`).
- Target `SemaFunctionBase*` (free function or class method) once resolved (`setFunction`).
- Optional `SemaErrorHandler*` for `fail/handle` constructs.
- `isNew()` convenience for constructor expressions.

Resolution steps:
1. Resolver gathers argument expressions and resolves each to a `SemaExpr*` and `SemaType*`.
2. `ResolveCallArgs` produces a `SmallVector<SemaType*, 8>` used for overload matching.
3. Lookup order: explicit parent namespace/class (`ResolveChild` overloads) → current scope function symbols (`Registry::LookupFunction`). Constructors are treated as methods with `MethodKind::METHOD_CONSTRUCTOR`.
4. Once a matching `SemaFunctionBase` is found, the call’s `SemaType` becomes the target’s return type (or the class type for constructors). If resolution fails, the call receives `SemaBuiltin::getErrorType()` to keep validation running.

## 10. Expressions & SemaExpr Chain
Every expression node (`ASTExpr`) holds a pointer to its semantic counterpart (`SemaExpr` subclasses such as `SemaVar`, `SemaCall`, `SemaValue`). Resolver ensures parent/child relationships are mirrored in the semantic tree, enabling later rewrites and diagnostics.

## 11. Functions, Classes, Enums in Sema
- **Functions**: `SemaFunction` couples an `ASTFunction` with its module, symbol table, comment, visibility, and `CodeGenFunction*`. Parameters and locals are added through `addParam`/`addLocalVar` as the resolver visits declarations.
- **Class Methods**: `SemaClassMethod` extends `SemaFunctionBase` with owning class, `this` instance, visibility, static flag, overridden method, comment, and `CodeGenClassMethod*`.
- **Classes**: `SemaClassType` (see §5), plus `SemaClassAttribute` for fields and `SemaClassInstance` for `this`.
- **Enums**: `SemaEnumType` with entry nodes (`SemaEnumEntry`).

## 12. Scope & Symbol Resolution Flow
1. **Module Entry**: Resolver creates a new `SemaModule`, registers it, sets namespace and scope, and visits top-level nodes in order. Namespaces must appear first.
2. **Imports**: `SemaImport` nodes are created. Symbols for imported namespaces are not immediately inserted; instead, resolver stores symbol tables for deferred lookup.
3. **Global Vars / Functions / Classes / Enums**: For each declaration, resolver creates the Sema object, inserts a symbol in the parent scope, then resolves nested content (e.g., function body, class members) under a pushed scope.
4. **Statements & Expressions**: Blocks call `EnterScope`/`ExitScope`. Locals are inserted into block-level symbol tables before resolving their initializers. Expressions delegate to `ResolveExpr`, `ResolveParent`, and `ResolveChild*` helpers.
5. **Errors**: Diagnostics are emitted through `Diag`. When a binding fails, resolver attaches `SemaErrorType` or `SemaVarKind::ERROR_VAR` placeholders so the pass can continue.

## 13. Worked Example: Local Variable Reference
1. Parser builds `ASTLocalVar` for `let x: int = 1;` and later an `ASTIdentifier` for `x`.
2. Resolver visits the declaration: creates `SemaLocalVar`, sets its `SemaType` to `SemaBuiltin::getIntType()`, inserts a `Symbol` `{ Name: "x", Kind: VAR, Ref: SemaLocalVar* }` into the block’s `SymbolTable`.
3. When resolving the identifier, `ResolveParent(ASTIdentifier*)` walks `CurrentScope` chain to find `x`, returning the `SemaLocalVar`. The identifier’s `SemaExpr*` becomes that `SemaVar`, and its `Type` pointer now references the underlying `SemaType`.

## 14. Complete AST Header Reference
The table below lists every header in `include/AST/` and the primary constructs it defines.

| Header | Key Types / Responsibilities | Notes |
|--------|------------------------------|-------|
| `ASTArg.h` | `ASTArg` | Represents positional call arguments with index bookkeeping. |
| `ASTAssignStmt.h` | `ASTAssignStmt` hierarchy | Assignment statements for `=` and compound ops. |
| `ASTAttribute.h` | `ASTAttribute` | Class/struct field declarations linked to `SemaClassAttribute`. |
| `ASTBase.h` | `ASTBase`, `ASTKind` | Base mixed into every AST node for location/kind. |
| `ASTBlockStmt.h` | `ASTBlockStmt` | Statement sequencing plus local variable registry. |
| `ASTBreakStmt.h` | `ASTBreakStmt` | `break` statement AST. |
| `ASTBuilder*.h` | `ASTBuilder`, `ASTBuilderStmt`, `ASTBuilderIfStmt`, `ASTBuilderLoopStmt`, `ASTBuilderSwitchStmt` | Parsing-time helpers that manufacture AST nodes for different syntactic categories. |
| `ASTCall.h` | `ASTCall`, `ASTCallKind` | Call expressions, including `new`/`new_shared` variants and argument storage. |
| `ASTCast.h` | `ASTCast` | Explicit cast expressions with target `ASTType`. |
| `ASTClass.h` | `ASTClass`, `ASTClassKind` | Class/interface/struct declarations storing modifiers, members, and bases. |
| `ASTComment.h` | `ASTComment` | Doc-block/lint comment capture feeding into `SemaComment`. |
| `ASTContinueStmt.h` | `ASTContinueStmt` | `continue` statement AST. |
| `ASTDeleteStmt.h` | `ASTDeleteStmt` | `delete` statement AST for memory/resource cleanup. |
| `ASTEnum.h` | `ASTEnum` | Enum declarations including modifiers and base types. |
| `ASTEnumEntry.h` | `ASTEnumEntry` | Individual enum entries tied to `SemaEnumEntry`. |
| `ASTExpr.h` | `ASTExpr`, `ASTExprKind` | Base for all expressions, tracks semantic attachments. |
| `ASTExprStmt.h` | `ASTExprStmt` | Statement wrapper holding a standalone expression. |
| `ASTFailStmt.h` | `ASTFailStmt` | `fail` control-flow statement (exception-like semantics). |
| `ASTFunction.h` | `ASTFunction`, `ASTFunctionKind` | Free function definitions with signature and body nodes. |
| `ASTHandleStmt.h` | `ASTHandleStmt` | `handle` blocks associated with `fail`/`error` handling. |
| `ASTIdentifier.h` | `ASTIdentifier` | Bare identifier references that resolve to `SemaVar`. |
| `ASTIfStmt.h` | `ASTIfStmt`, `ASTIfBlock` | `if`/`elseif`/`else` constructs with nested blocks. |
| `ASTImport.h` | `ASTImport` | Qualified import statements and aliases. |
| `ASTLocalVar.h` | `ASTLocalVar` | Local variable declarations bridging to `SemaLocalVar`. |
| `ASTLoopInStmt.h` | `ASTLoopInStmt` | `for-in` style looping construct. |
| `ASTLoopStmt.h` | `ASTLoopStmt` | Traditional loop statements (while/for). |
| `ASTMember.h` | `ASTMember` | Member access expressions that link to `SemaMemberVar`. |
| `ASTMethod.h` | `ASTMethod` | Class-scoped function definitions. |
| `ASTModifier.h` | `ASTModifier`, `ASTModifierKind` | Encodes `public/private/protected/static/const`. |
| `ASTModule.h` | `ASTModule` | Translation unit root storing namespace and toplevel node order. |
| `ASTName.h` | `ASTName` | Qualified identifier segment used by imports/types. |
| `ASTNameSpace.h` | `ASTNameSpace` | Namespace declarations referencing `ASTName` chains. |
| `ASTNode.h` | `ASTNode` | Base class for visitable nodes with `Visited` flag. |
| `ASTOp.h` | `ASTUnaryOp`, `ASTBinaryOp`, `ASTTernaryOp`, `Precedence` and enum variants | All operator expressions and precedence utilities. |
| `ASTParam.h` | `ASTParam` | Function/method parameter declaration nodes. |
| `ASTReturnStmt.h` | `ASTReturnStmt` | `return` statement AST. |
| `ASTRuleStmt.h` | `ASTRuleStmt` | Rule-based statement used by pattern/DSL features. |
| `ASTStmt.h` | `ASTStmt`, `ASTStmtKind` | Base for statements with parent/function tracking. |
| `ASTSwitchStmt.h` | `ASTSwitchStmt`, helper block types | `switch`/`case` representation. |
| `ASTType.h` | `ASTType`, `ASTBuiltinType`, `ASTNamedType`, `ASTArrayType`, enums | Type syntax nodes and built-in identifiers. |
| `ASTValue.h` | `ASTValue` hierarchy (`ASTBoolValue`, `ASTNumberValue`, ... ) | Literal expressions and aggregate literal forms. |
| `ASTVar.h` | `ASTVar`, `ASTVarKind` | Base for all variable declarations. |
| `ASTVisitor.h` | `ASTVisitor` | Visitor interface used by resolver and other passes. |

## 15. Complete Sema Header Reference
All headers in `include/Sema/` are catalogued below with their primary exports.

| Header | Key Types / Responsibilities | Notes |
|--------|------------------------------|-------|
| `Helper.h` | Utility helpers | Shared resolver/builder helpers (string munging, diagnostics glue). |
| `Registry.h` | `Registry`, `LocalScope` | Global registry of modules, namespaces, builtin scope, and function bodies. |
| `Resolver.h` | `Resolver` | AST visitor driving semantic binding and scope management. |
| `Sema.h` | `Sema` | Entry point coordinating builder, resolver, validator, diagnostics, and context. |
| `SemaBuilder.h` | `SemaBuilder` | Factory/static creators for every Sema node class. |
| `SemaBuilderModifiers.h` | Modifier helpers | Converts `ASTModifier`s into semantic visibility/const/static flags. |
| `SemaBuiltin.h` | `SemaBuiltin` | Singleton accessors for builtin `SemaType`s and array type creation. |
| `SemaCall.h` | `SemaCall` | Semantic representation of call expressions (see §9). |
| `SemaClassAttribute.h` | `SemaClassAttribute` | Semantic record for declared class members including visibility/statics. |
| `SemaClassInstance.h` | `SemaClassInstance` | Synthetic `this` variable plus base-instance mapping. |
| `SemaClassMethod.h` | `SemaClassMethod`, `SemaClassMethodKind` | Class methods/ctors/abstract functions extending `SemaFunctionBase`. |
| `SemaClassType.h` | `SemaClassType`, `SemaClassKind` | Full semantic class metadata (symbols, bases, members, codegen hooks). |
| `SemaComment.h` | `SemaComment` | Wraps `ASTComment` for later documentation use. |
| `SemaEnumEntry.h` | `SemaEnumEntry` | Semantic enum constants with indices/codegen link. |
| `SemaEnumType.h` | `SemaEnumType` | Enum semantic metadata (symbols, entries, bases). |
| `SemaErrorHandler.h` | `SemaErrorHandler` | Tracks `fail/handle` constructs associated with calls and functions. |
| `SemaExpr.h` | `SemaExpr` | Semantic base for all expressions with parent/child chain. |
| `SemaFunction.h` | `SemaFunction` | Free-function semantic node with module/scope/comment info. |
| `SemaFunctionBase.h` | `SemaFunctionBase` | Shared functionality across `SemaFunction` and `SemaClassMethod`. |
| `SemaImport.h` | `SemaImport` | Semantic record of `ASTImport`, including symbol table linkage. |
| `SemaLocalVar.h` | `SemaLocalVar` | Local variable semantics with `CodeGenVar*`. |
| `SemaMemberVar.h` | `SemaMemberVar` | Member-access semantic nodes bridging expressions to attributes. |
| `SemaModule.h` | `SemaModule` | Ties an `ASTModule` to namespace, imports, and child nodes. |
| `SemaNameSpace.h` | `SemaNameSpace` | Namespace-level semantic scope, child hierarchy, and global symbol maps. |
| `SemaNode.h` | `SemaNode`, `SemaKind` | Base for all semantic constructs. |
| `SemaParam.h` | `SemaParam` | Function/method parameter semantics with codegen handle. |
| `SemaType.h` | `SemaType` hierarchy (`SemaIntType`, `SemaFloatType`, `SemaArrayType`, `SemaErrorType`) | Full type system semantics (see §5). |
| `SemaValidator.h` | `SemaValidator` | Post-resolution validation pass. |
| `SemaValue.h` | `SemaValue` hierarchy (`SemaBoolValue`, `SemaIntValue`, etc.) | Semantic literal values. |
| `SemaVar.h` | `SemaVar`, `SemaVarKind` | Base for semantic variables (see §7). |
| `SemaVisibilityKind.h` | `SemaVisibilityKind` | Enum for `private/protected/default/public`. |
| `Symbol.h` | `Symbol` | Name → `SemaNode` binding stored in symbol tables. |
| `SymbolTable.h` | `SymbolTable` | Scope stack implementation used throughout resolution. |

This reference should be treated as the authoritative description of Fly’s semantic graph up to, but not including, CodeGen.
