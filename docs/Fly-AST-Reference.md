# Fly AST & Sema Reference

This document describes every Abstract Syntax Tree (AST) construct produced by the Fly parser and how those nodes are translated into Semantic Analysis (Sema) objects before the CodeGen pass. It includes detailed examples from the test suite and concentrates on symbol table formation and on the `SemaType`, `SemaVar`, `SemaCall`, and `SemaMember*` families that participate in symbol resolution.

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

## 3. Detailed AST Class Reference

### 3.1 Module, Namespace, and Import

#### ASTModule
**Purpose**: Root container for a single translation unit/source file.

**Location**: `include/AST/ASTModule.h`, `src/AST/ASTModule.cpp`

**Structure**:
- **`File`**: `InputFile*` - pointer to the input source file
- **`Name`**: `std::string` - module filename
- **`Header`**: `bool` - true if this is a header file
- **`NameSpace`**: Optional `ASTNameSpace*` declaring the module's namespace
- **`Nodes`**: `SmallVector<ASTNode*, 8>` of top-level declarations (imports, functions, classes, enums, global variables)

**Key Methods**:
- `getFile()`: Returns the `InputFile*` pointer
- `getName()`: Returns the module filename
- `isHeader()`: Returns true if this is a header file
- `getNameSpace()`: Returns the namespace declaration (can be null for default namespace)
- `setNameSpace(ASTNameSpace*)`: Sets the namespace declaration
- `getNodes()`: Returns vector of all top-level declarations
- `addNode(ASTNode*)`: Adds a top-level declaration to the module

**Parser Behavior**:
1. Module creation begins when parsing starts
2. First checks for optional `namespace` declaration
3. Parses imports (must come before other declarations)
4. Parses top-level nodes (classes, enums, functions, global vars) in order
5. Maintains declaration order in the `Nodes` vector

**Test Example** (`ParserNameSpaceTest.cpp`):
```cpp
// Source:
namespace com.test
import other.Module
void main() {}

// AST Structure:
ASTModule
  ├─ NameSpace: ASTNameSpace("com.test")
  ├─ Nodes[0]: ASTImport("other.Module")
  └─ Nodes[1]: ASTFunction("main")
```

#### ASTNameSpace
**Purpose**: Represents a namespace declaration at the module level.

**Location**: `include/AST/ASTNameSpace.h`, `src/AST/ASTNameSpace.cpp`

**Structure**:
- **`Names`**: `SmallVector<ASTName*, 4>` - sequence of name segments forming the qualified namespace path (e.g., `com.example.project` becomes a vector of 3 ASTName objects)

**Syntax**: `namespace Identifier ['.' Identifier]*`

**Key Points**:
- Must be the first declaration in a module (if present)
- Creates a hierarchical namespace structure
- Each segment separated by `.` creates a nested namespace level

**Test Example**:
```cpp
// Single-level namespace
namespace myapp
// → Names: [ASTName("myapp")]

// Multi-level namespace  
namespace com.example.app
// → Names: [ASTName("com"), ASTName("example"), ASTName("app")]
```

#### ASTImport
**Purpose**: Imports symbols from other modules/namespaces.

**Location**: `include/AST/ASTImport.h`, `src/AST/ASTImport.cpp`

**Structure**:
- **`Names`**: `SmallVector<ASTName*, 4>` - qualified name path of module/namespace to import
- **`Alias`**: `SmallVector<ASTName*, 4>` - optional alias name path

**Syntax**: `import QualifiedName ['as' Alias]`

**Parser Behavior**:
- Imports must appear after namespace declaration but before other top-level nodes
- Multiple imports are allowed
- Alias is optional

**Test Example**:
```cpp
// Simple import
import std.io
// → ASTName: "std.io", Alias: empty

// Import with alias
import std.collections as col
// → ASTName: "std.collections", Alias: "col"
```

**Resolution**:
- Resolver looks up the imported namespace in the Registry
- Creates `SemaImport` with reference to target namespace's symbol table
- Symbols from imported namespace become available in current scope

#### ASTName
**Purpose**: Represents a single identifier segment in a qualified name.

**Location**: `include/AST/ASTName.h`, `src/AST/ASTName.cpp`

**Structure**:
- **`Name`**: `llvm::StringRef` - single identifier string

**Note**: Extends `ASTBase` (not `ASTNode`). Qualified names like `com.example.MyClass` are represented as vectors of multiple `ASTName` objects, each containing one segment.

**Usage**: Used by namespaces, imports, and qualified type references.

**Example**:
```cpp
com.example.MyClass
// → Vector of 3 ASTName objects:
//    [0]: ASTName(Name="com")
//    [1]: ASTName(Name="example")
//    [2]: ASTName(Name="MyClass")
```

### 3.2 Types

#### ASTType Hierarchy
**Base Class**: `ASTType` (`include/AST/ASTType.h`)

**Type Kinds** (`ASTTypeKind` enum):
- `TYPE_BUILTIN` - Built-in primitive types
- `TYPE_NAMED` - User-defined types (classes, enums)
- `TYPE_ARRAY` - Array types with optional size

**Common Fields**:
- **`TypeKind`**: Identifies the concrete type subclass
- **`Sema`**: `SemaType*` pointer (resolved type)

#### ASTBuiltinType
**Purpose**: Represents primitive/built-in types.

**Location**: `include/AST/ASTType.h`, `src/AST/ASTType.cpp`

**Type Keywords** (`ASTBuiltinTypeKind`):
- **`TYPE_BOOL`**: Boolean type (`bool`)
- **`TYPE_BYTE`**: 8-bit unsigned integer (`byte`)
- **`TYPE_USHORT`**: 16-bit unsigned integer (`ushort`)
- **`TYPE_SHORT`**: 16-bit signed integer (`short`)
- **`TYPE_UINT`**: 32-bit unsigned integer (`uint`)
- **`TYPE_INT`**: 32-bit signed integer (`int`)
- **`TYPE_ULONG`**: 64-bit unsigned integer (`ulong`)
- **`TYPE_LONG`**: 64-bit signed integer (`long`)
- **`TYPE_FLOAT`**: 32-bit floating point (`float`)
- **`TYPE_DOUBLE`**: 64-bit floating point (`double`)
- **`TYPE_STRING`**: String type (`string`)
- **`TYPE_VOID`**: Void type (`void`) - used for functions with no return value
- **`TYPE_ERROR`**: Error type (`error`) - used in error handling

**Test Example** (`ParserLocalVarTest.cpp`):
```cpp
void func() {
    bool a = false      // ASTBuiltinType(TYPE_BOOL)
    int e = 0          // ASTBuiltinType(TYPE_INT)
    float i = 0.0      // ASTBuiltinType(TYPE_FLOAT)
    double j = 0.0     // ASTBuiltinType(TYPE_DOUBLE)
    string s = "hello" // ASTBuiltinType(TYPE_STRING)
}
```

#### ASTNamedType
**Purpose**: References user-defined types (classes, enums, structs, interfaces).

**Structure**:
- **`Names`**: `SmallVector<ASTName*, 4>` - qualified type name path (can include namespace segments)

**Examples**:
```cpp
MyClass obj           // Names: [ASTName("MyClass")]
com.example.Type t    // Names: [ASTName("com"), ASTName("example"), ASTName("Type")]
```

**Resolution**: Resolver looks up the name path in the current scope and imported namespaces to find the corresponding `SemaClassType` or `SemaEnumType`.

#### ASTArrayType
**Purpose**: Represents array types with optional compile-time size.

**Structure**:
- **`Type`**: `ASTType*` - element type
- **`Size`**: `ASTExpr*` - optional size expression (null for dynamic arrays)

**Syntax Variations**:
```cpp
int[] arr           // Dynamic array: Size = null
int[10] arr         // Fixed size: Size = ASTNumberValue("10")
MyClass[] objects   // Array of user-defined type
```

**Test Example** (`ParserLocalVarTest.cpp`):
```cpp
void func() {
    byte[] a                    // ASTArrayType(Type=byte, Size=null)
    byte[] b = {}              // Empty array initializer
    byte[] c = {1, 2, 3}       // Array with values
    byte[3] d                  // ASTArrayType(Type=byte, Size=3)
    byte[3] e = {1, 2, 3}      // Fixed-size with initializer
}
```

### 3.3 Values & Literals

#### ASTValue Hierarchy
**Base Class**: `ASTValue` extends `ASTExpr` (`include/AST/ASTValue.h`)

**Value Kinds** (`ASTValueKind`):
- `VAL_BOOL` - Boolean literals
- `VAL_NUMBER` - Numeric literals (integers and floats)
- `VAL_STRING` - String literals  
- `VAL_ARRAY` - Array literals
- `VAL_STRUCT` - Struct initialization literals
- `VAL_NULL` - Null literal
- `VAL_DEFAULT` - Default value placeholder

**Common Fields**:
- **`ValueKind`**: Identifies concrete value type
- **`Sema`**: `SemaValue*` pointer

**Predicates**:
- `isBool()`, `isNumber()`, `isString()`, `isArray()`, `isStruct()`, `isNull()`, `isDefault()`

#### ASTBoolValue
**Purpose**: Boolean literal values.

**Structure**:
- **`Value`**: `bool` - true or false

**Examples**:
```cpp
bool a = true
bool b = false
```

#### ASTNumberValue  
**Purpose**: Numeric literals (integers and floating-point).

**Structure**:
- **`Value`**: `llvm::StringRef` - raw textual representation

**Key Points**:
- Stores number as string to preserve precision
- Parser doesn't distinguish int vs float at AST level
- Semantic analysis determines actual numeric type based on context

**Examples**:
```cpp
int i = 42        // Value = "42"
float f = 3.14    // Value = "3.14"
long l = 1000000  // Value = "1000000"
```

#### ASTStringValue
**Purpose**: String literal values.

**Structure**:
- **`Value`**: `llvm::StringRef` - string content (without quotes)

**Special Cases**:
```cpp
string empty = ""    // Value = "" (empty string)
string text = "hello" // Value = "hello"
```

**Test Example** (`ParserLocalVarTest.cpp`):
```cpp
void func() {
    string a = ""        // Empty string
    string b = "test"    // Regular string
}
```

#### ASTArrayValue
**Purpose**: Array literal initialization.

**Structure**:
- **`Values`**: `SmallVector<ASTValue*, 8>` - element values

**Syntax**: `'{' [Value (',' Value)*] '}'`

**Examples**:
```cpp
int[] arr = {}              // Empty: Values.size() == 0
int[] nums = {1, 2, 3}      // Three elements
byte[] chars = {'a', 'b'}   // Character array
```

**Test Example**:
```cpp
void func() {
    byte[] a = {}                    // Empty array
    byte[] b = {1, 2, 3}            // Values: [1, 2, 3]
    byte[] c = {'a', 'b', 'c'}      // Character values
}
```

#### ASTStructValue
**Purpose**: Struct/object initialization with named fields.

**Structure**:
- **`Values`**: `llvm::StringMap<ASTValue*>` - field name → value mapping

**Syntax**: `'{' [Field '=' Value (',' Field '=' Value)*] '}'`

**Example** (`ParserClassTest.cpp`):
```cpp
Test x = { a = 3, b = 1 }
// ASTStructValue:
//   Values["a"] = ASTNumberValue("3")
//   Values["b"] = ASTNumberValue("1")
```

**Access**:
- `getValues()`: Returns the StringMap
- `size()`: Number of fields
- `empty()`: True if no fields specified

#### ASTNullValue
**Purpose**: Null literal for reference types.

**Example**:
```cpp
MyClass obj = null
string str = null
```

#### ASTDefaultValue
**Purpose**: Implicit default value (used when no initializer provided).

**Example**:
```cpp
int x    // Implicitly: x = default (0 for int)
```

### 3.4 Expressions

#### ASTExpr Base
**Purpose**: Base class for all expressions.

**Location**: `include/AST/ASTExpr.h`

**Expression Kinds** (`ASTExprKind`):
- `EXPR_VALUE` - Literal values
- `EXPR_IDENTIFIER` - Variable/parameter references
- `EXPR_MEMBER` - Member access (`.` operator)
- `EXPR_CALL` - Function/method calls
- `EXPR_UNARY` - Unary operators
- `EXPR_BINARY` - Binary operators
- `EXPR_TERNARY` - Ternary conditional operator
- `EXPR_CAST` - Type casting

**Common Fields**:
- **`ExprKind`**: Identifies concrete expression type
- **`Parent`**: `ASTExpr*` - parent expression in tree
- **`Child`**: `ASTExpr*` - child expression (for chaining)
- **`Sema`**: `SemaExpr*` - semantic representation (provides type information)

#### ASTIdentifier
**Purpose**: References to variables, parameters, or named entities.

**Structure**:
- **`Name`**: `llvm::StringRef` - identifier name
- **`Var`**: `ASTVar*` - referenced variable declaration (may be set during parsing)
- **`Sema`**: `SemaVar*` - semantic representation of the referenced entity

**Examples**:
```cpp
int a = 5
int b = a    // ASTIdentifier("a") → references the variable 'a'

void func(int param) {
    int x = param  // ASTIdentifier("param")
}
```

**Important**: When an identifier appears on the left side of an assignment within an `ASTExprStmt`, it represents the l-value (storage location). The same identifier on the right side is an r-value (reads the value).

#### ASTMember
**Purpose**: Member access expressions using the dot (`.`) operator.

**Structure**:
- **`Name`**: `llvm::StringRef` - member field/method name
- **`Parent`**: `ASTExpr*` - expression before the dot (inherited from `ASTExpr` base class)

**Syntax**: `Expression '.' Identifier`

**Key Rule**: The last part of a member access chain is always the `ASTMember`, and the parent is the preceding expression.

**Examples**:
```cpp
obj.field           // ASTMember(Name="field", Parent=ASTIdentifier("obj"))
Test.A              // ASTMember(Name="A", Parent=ASTIdentifier("Test"))
a.b.c              // ASTMember(Name="c", Parent=ASTMember(Name="b", Parent=ASTIdentifier("a")))
```

**Test Example** (`ParserClassTest.cpp - Enum test`):
```cpp
Test a = Test.A
// ASTMember:
//   Name = "A"
//   Parent = ASTIdentifier("Test")
//   Resolves to enum entry 'A' of enum 'Test'
```

**Resolution**:
1. Parser creates `ASTMember` with name and parent expression
2. Resolver evaluates parent to get its type
3. Looks up member in parent's symbol table (class attributes, enum entries, etc.)
4. Creates appropriate `SemaExpr` subclass (e.g., `SemaMember`) linking to the member definition

#### ASTCall
**Purpose**: Function calls, method invocations, and constructor calls.

**Structure**:
- **`Name`**: `llvm::StringRef` - function/method name
- **`Parent`**: `ASTExpr*` - for method calls (object instance), inherited from `ASTExpr`
- **`Args`**: `SmallVector<ASTArg*, 8>` - call arguments
- **`CallKind`**: `ASTCallKind` enum
  - `CALL_DIRECT` - Regular function/method call
  - `CALL_NEW` - Constructor with `new` keyword
  - `CALL_NEW_UNIQUE` - Unique pointer constructor
  - `CALL_NEW_SHARED` - Shared pointer constructor
  - `CALL_NEW_WEAK` - Weak pointer constructor

**Syntax Variations**:
```cpp
func()              // Function call: Parent = null, CallKind = CALL_DIRECT
obj.method()        // Method call: Parent = ASTIdentifier("obj"), CallKind = CALL_DIRECT
new Class()         // Constructor: CallKind = CALL_NEW
new_shared Class()  // Shared constructor: CallKind = CALL_NEW_SHARED
```

**Test Example**:
```cpp
void func1() {
    Test t = new Test()
}
// ASTCall:
//   Name = "Test"
//   CallKind = CALL_NEW
//   Args = []
```

#### ASTArg
**Purpose**: Represents a single argument in a function call.

**Structure**:
- **`Index`**: Position in argument list
- **`Expr`**: `ASTExpr*` - argument expression

**Example**:
```cpp
func(a, b + 1, "hello")
// Args:
//   [0]: ASTIdentifier("a")
//   [1]: ASTBinary(OP_BINARY_ARITH_ADD, left=b, right=1)
//   [2]: ASTStringValue("hello")
```

#### ASTUnary
**Purpose**: Unary operator expressions.

**Structure**:
- **`OpKind`**: `ASTUnaryKind`
  - `OP_UNARY_PRE_INCR` - Prefix increment `++a`
  - `OP_UNARY_POST_INCR` - Postfix increment `a++`
  - `OP_UNARY_PRE_DECR` - Prefix decrement `--a`
  - `OP_UNARY_POST_DECR` - Postfix decrement `a--`
  - `OP_UNARY_NOT_LOG` - Logical not `!a`
- **`Expr`**: `ASTExpr*` - operand expression
- **`OpLocation`**: Source location of operator

**Test Example** (`ParserExprTest.cpp`):
```cpp
void func(int a) {
    ++a     // OP_UNARY_PRE_INCR
    a++     // OP_UNARY_POST_INCR
    --a     // OP_UNARY_PRE_DECR
    a--     // OP_UNARY_POST_DECR
}
```

**Complex Example**:
```cpp
a = a++ + ++a
// ASTBinary(OP_BINARY_ASSIGN)
//   left: ASTIdentifier("a")
//   right: ASTBinary(OP_BINARY_ARITH_ADD)
//     left: ASTUnary(OP_UNARY_POST_INCR, ASTIdentifier("a"))
//     right: ASTUnary(OP_UNARY_PRE_INCR, ASTIdentifier("a"))
```

#### ASTBinary
**Purpose**: Binary operator expressions (arithmetic, logical, comparison, assignment).

**Structure**:
- **`OpKind`**: `ASTBinaryKind` (see detailed list below)
- **`LeftExpr`**: `ASTExpr*` - left operand
- **`RightExpr`**: `ASTExpr*` - right operand
- **`OpLocation`**: Source location of operator

**Operator Categories**:

**Arithmetic Operators**:
- `OP_BINARY_ARITH_ADD` (`+`), `OP_BINARY_ARITH_SUB` (`-`), `OP_BINARY_ARITH_MUL` (`*`), `OP_BINARY_ARITH_DIV` (`/`), `OP_BINARY_ARITH_MOD` (`%`)

**Bitwise Operators**:
- `OP_BINARY_ARITH_AND` (`&`), `OP_BINARY_ARITH_OR` (`|`), `OP_BINARY_ARITH_XOR` (`^`)
- `OP_BINARY_ARITH_SHIFT_L` (`<<`), `OP_BINARY_ARITH_SHIFT_R` (`>>`)

**Logical Operators**:
- `OP_BINARY_LOGIC_AND` (`&&`), `OP_BINARY_LOGIC_OR` (`||`)

**Comparison Operators**:
- `OP_BINARY_COMPARE_EQ` (`==`) - Equality comparison
- `OP_BINARY_COMPARE_NE` (`!=`) - Not equal
- `OP_BINARY_COMPARE_GT` (`>`), `OP_BINARY_COMPARE_GTE` (`>=`)
- `OP_BINARY_COMPARE_LT` (`<`), `OP_BINARY_COMPARE_LTE` (`<=`)

**Assignment Operators**:
- `OP_BINARY_ASSIGN` (`=`) - **CRITICAL**: Simple assignment, NOT equality
- `OP_BINARY_ASSIGN_ADD` (`+=`), `OP_BINARY_ASSIGN_SUB` (`-=`)
- `OP_BINARY_ASSIGN_MUL` (`*=`), `OP_BINARY_ASSIGN_DIV` (`/=`), `OP_BINARY_ASSIGN_MOD` (`%=`)
- `OP_BINARY_ASSIGN_AND` (`&=`), `OP_BINARY_ASSIGN_OR` (`|=`), `OP_BINARY_ASSIGN_XOR` (`^=`)
- `OP_BINARY_ASSIGN_SHIFT_L` (`<<=`), `OP_BINARY_ASSIGN_SHIFT_R` (`>>=`)

**CRITICAL DISTINCTION: Assignment (`=`) vs Equality (`==`)**

The parser creates different `ASTBinaryKind` values for these two operators:
- **`OP_BINARY_ASSIGN`**: The assignment operator `=` (stores value)
- **`OP_BINARY_COMPARE_EQ`**: The equality comparison `==` (returns boolean)

**Assignment Example** (`ParserExprTest.cpp`):
```cpp
void func(int a) {
    a = a + 1
}
// AST Structure:
// ASTExprStmt
//   └─ expr: ASTBinary(OP_BINARY_ASSIGN)  ← Assignment operator
//       ├─ left: ASTIdentifier("a")         ← L-value
//       └─ right: ASTBinary(OP_BINARY_ARITH_ADD) ← R-value expression
//            ├─ left: ASTIdentifier("a")
//            └─ right: ASTNumberValue("1")
```

**Equality Comparison Example**:
```cpp
void func(bool result, int a) {
    result = a == 5
}
// ASTExprStmt
//   └─ expr: ASTBinary(OP_BINARY_ASSIGN)
//       ├─ left: ASTIdentifier("result")
//       └─ right: ASTBinary(OP_BINARY_COMPARE_EQ)  ← Equality comparison
//            ├─ left: ASTIdentifier("a")
//            └─ right: ASTNumberValue("5")
```

**Compound Assignment Example**:
```cpp
a += 1   // ASTBinary(OP_BINARY_ASSIGN_ADD, left=a, right=1)
a -= 1   // ASTBinary(OP_BINARY_ASSIGN_SUB, left=a, right=1)
```

**Precedence**: The `Precedence` enum defines operator precedence:
- `LOWEST` - No operators
- `ASSIGNMENT` - `=`, `+=`, `-=`, etc.
- `TERNARY` - `? :`
- `LOGICAL` - `||`, `&&`
- `RELATIONAL` - `==`, `!=`, `<`, `>`, `<=`, `>=`
- `ADDITIVE` - `+`, `-`
- `MULTIPLICATIVE` - `*`, `/`, `%`
- `UNARY` - Unary operators
- `PRIMARY` - Literals, identifiers, calls

#### ASTTernary
**Purpose**: Conditional ternary operator.

**Structure**:
- **`ConditionExpr`**: `ASTExpr*` - condition to evaluate
- **`TrueOpLocation`**: `SourceLocation` - location of `?` operator
- **`TrueExpr`**: `ASTExpr*` - result if condition is true
- **`FalseOpLocation`**: `SourceLocation` - location of `:` operator
- **`FalseExpr`**: `ASTExpr*` - result if condition is false

**Syntax**: `Condition '?' TrueExpr ':' FalseExpr`

**Test Example** (`ParserExprTest.cpp`):
```cpp
void func(int a) {
    a = a==1 ? 1 : a
}
// ASTTernary:
//   ConditionExpr: ASTBinary(OP_BINARY_COMPARE_EQ, left=a, right=1)
//   TrueExpr: ASTNumberValue("1")
//   FalseExpr: ASTIdentifier("a")
```

**Note**: The entire ternary expression can be embedded in an assignment:
```cpp
result = condition ? value1 : value2
// The ternary is the right-hand side of the assignment
```

#### ASTCast
**Purpose**: Explicit type conversion.

**Structure**:
- **`Type`**: `ASTType*` - target type
- **`Expr`**: `ASTExpr*` - expression to cast

**Syntax**: `'(' Type ')' Expression`

**Example**:
```cpp
float f = (float)intValue
```

### 3.5 Statements

#### ASTStmt Base
**Purpose**: Base class for all statements.

**Location**: `include/AST/ASTStmt.h`

**Statement Kinds** (`ASTStmtKind`):
- `STMT_BLOCK` - Block of statements
- `STMT_EXPR` - Expression statement
- `STMT_IF` - If/elsif/else conditionals
- `STMT_SWITCH` - Switch/case statement
- `STMT_LOOP` - While/for loops
- `STMT_LOOP_IN` - For-in loops
- `STMT_RETURN` - Return statement
- `STMT_BREAK` - Break statement
- `STMT_CONTINUE` - Continue statement
- `STMT_DELETE` - Delete/cleanup statement
- `STMT_FAIL` - Error propagation
- `STMT_HANDLE` - Error handling block
- `STMT_RULE` - Rule-based statement

**Common Fields**:
- **`StmtKind`**: Identifies concrete statement type
- **`Parent`**: `ASTStmt*` - containing statement
- **`Function`**: `ASTFunction*` - containing function

#### ASTBlockStmt
**Purpose**: Sequence of statements with local scope.

**Structure**:
- **`Content`**: `SmallVector<ASTStmt*, 16>` - ordered statements
- **`LocalVars`**: `llvm::StringMap<ASTLocalVar*>` - local variables declared in this block
- **`Parent`**: `ASTStmt*` - parent block or null for function body

**Special Blocks**:
- **Function Body**: Created by `ASTBuilder::CreateBody()`, marked with `BLOCK_BODY`
- **Regular Block**: Created by `ASTBuilder::CreateBlockStmt()`

**Example**:
```cpp
void func() {           // Function body block
    int a = 1;          // Content[0]
    {                   // Nested block
        int b = 2;      // Content[0] of nested block
    }
    int c = 3;          // Content[1] of function body
}
```

**Test Example** (`ParserBlockTest.cpp`):
```cpp
void func(int a, int b) {
    if (a == 1) {
        b = 0
    }
}
// ASTFunction
//   └─ Body: ASTBlockStmt
//       └─ Content[0]: ASTIfStmt
//           └─ Stmt: ASTBlockStmt
//               └─ Content[0]: ASTExprStmt
```

#### ASTExprStmt
**Purpose**: Wraps an expression as a statement.

**Structure**:
- **`Expr`**: `ASTExpr*` - the expression

**Usage**: Used for:
- Assignments (via `OP_BINARY_ASSIGN`)
- Function calls
- Increment/decrement operations
- Any expression executed for side effects

**IMPORTANT**: Assignments are now represented using `ASTExprStmt` containing an `ASTBinary` with `OP_BINARY_ASSIGN`, not a separate `ASTAssignStmt` class.

**Example**:
```cpp
a = 5              // ASTExprStmt(Expr=ASTBinary(OP_BINARY_ASSIGN, ...))
func()             // ASTExprStmt(Expr=ASTCall("func"))
a++                // ASTExprStmt(Expr=ASTUnary(OP_UNARY_POST_INCR, ...))
```

#### ASTIfStmt
**Purpose**: Conditional branching with if/elsif/else.

**Structure**:
- **`Rule`**: `ASTExpr*` - condition expression for main `if`
- **`Stmt`**: `ASTStmt*` - statement/block to execute if condition true
- **`Elsif`**: `SmallVector<ASTRuleStmt*, 4>` - optional elsif clauses
- **`Else`**: `ASTStmt*` - optional else clause

**Syntax**:
```
'if' ['('] Condition [')'] Statement
['elsif' ['('] Condition [')'] Statement]*
['else' Statement]
```

**Test Example** (`ParserBlockTest.cpp`):
```cpp
void func(int a, int b) {
    if (a == 1) {
        b = 0
    } elsif (a == 2) {
        b = 1
    } else {
        b = 2
    }
}
// ASTIfStmt:
//   Rule: ASTBinary(OP_BINARY_COMPARE_EQ, left=a, right=1)
//   Stmt: ASTBlockStmt containing assignment b=0
//   Elsif[0]: ASTRuleStmt
//     Rule: ASTBinary(OP_BINARY_COMPARE_EQ, left=a, right=2)
//     Stmt: ASTBlockStmt containing assignment b=1
//   Else: ASTBlockStmt containing assignment b=2
```

**Inline Form** (without braces):
```cpp
if (a == 1) b = 0
elsif a == 2 b = 1  // Parentheses optional
else b = 2
```

#### ASTRuleStmt
**Purpose**: Represents a condition-statement pair (used in elsif, switch cases).

**Structure**:
- **`Rule`**: `ASTExpr*` - condition/case expression
- **`Stmt`**: `ASTStmt*` - statement to execute

**Usage**: Used by:
- `elsif` clauses in if statements
- `case` clauses in switch statements

#### ASTSwitchStmt
**Purpose**: Multi-way branch based on value.

**Structure**:
- **`Expr`**: `ASTExpr*` - expression to evaluate
- **`Cases`**: `SmallVector<ASTRuleStmt*, 8>` - case clauses
- **`Default`**: `ASTStmt*` - optional default clause

**Syntax**:
```
'switch' ['('] Expression [')'] '{'
  ('case' Value ':' Statement)*
  ['default' ':' Statement]
'}'
```

**Test Example** (`ParserBlockTest.cpp`):
```cpp
void func(int a) {
    switch (a) {
        case 1:
            break
        case 2:
        default:
            return
    }
}
// ASTSwitchStmt:
//   Expr: ASTIdentifier("a")
//   Cases[0]: ASTRuleStmt
//     Rule: ASTNumberValue("1")
//     Stmt: ASTBlockStmt with break
//   Cases[1]: ASTRuleStmt
//     Rule: ASTNumberValue("2")
//     Stmt: empty (falls through to default)
//   Default: ASTBlockStmt with return
```

**Fall-Through**: Case without statement falls through to next case or default.

#### ASTLoopStmt
**Purpose**: Traditional loops (while, for).

**Location**: `include/AST/ASTLoopStmt.h`

**Structure**: Extends `ASTRuleStmt`

**Fields**:
- **`Init`**: `ASTStmt*` - initialization statement (for-loops, null for while)
- **`Rule`**: `ASTExpr*` - loop condition (inherited from `ASTRuleStmt`)
- **`Post`**: `ASTStmt*` - post-iteration statement (for-loops update, null for while)
- **`Stmt`**: `ASTStmt*` - loop body (inherited from `ASTRuleStmt`)
- **`VerifyConditionAtEnd`**: `bool` - true for do-while loops

**Syntax Variations**:
```cpp
// While loop
while (condition) statement

// For loop
for (init; condition; post) statement

// Do-while loop
do statement while (condition)
```

**Example**:
```cpp
for (int i = 0; i < 10; i++) {
    // body
}
// ASTLoopStmt:
//   Init: local var declaration for 'i'
//   Rule: ASTBinary(OP_BINARY_COMPARE_LT, i, 10)
//   Post: ASTUnary(OP_UNARY_POST_INCR, i)
//   Stmt: ASTBlockStmt(body)
```

#### ASTLoopInStmt  
**Purpose**: For-in style iteration over collections.

**Structure**:
- **`Var`**: `ASTLocalVar*` - loop variable
- **`Expr`**: `ASTExpr*` - collection expression
- **`Stmt`**: `ASTStmt*` - loop body

**Syntax**: `'for' Variable 'in' Expression Statement`

**Example**:
```cpp
for (item in collection) {
    // process item
}
```

#### ASTReturnStmt
**Purpose**: Return from function with optional value.

**Structure**:
- **`Expr`**: `ASTExpr*` - return value (null for void functions)

**Examples**:
```cpp
return           // Expr = null (void return)
return value     // Expr = ASTIdentifier("value")
return a + 1     // Expr = ASTBinary(OP_BINARY_ARITH_ADD, ...)
```

#### ASTBreakStmt
**Purpose**: Exit from loop or switch.

**Structure**: No additional fields (marker statement)

**Usage**: Must appear inside loop or switch statement.

#### ASTContinueStmt
**Purpose**: Skip to next iteration of loop.

**Structure**: No additional fields (marker statement)

**Usage**: Must appear inside loop statement.

#### ASTDeleteStmt
**Purpose**: Explicit resource cleanup/deallocation.

**Structure**:
- **`Expr`**: `ASTExpr*` - expression to delete

**Example**:
```cpp
delete obj
```

#### ASTFailStmt
**Purpose**: Error propagation (fail/throw semantics).

**Structure**:
- **`Expr`**: `ASTExpr*` - optional error payload

**Syntax**: `'fail' [Expression]`

**Examples**:
```cpp
fail                  // Expr = null (void failure)
fail 404              // Expr = ASTNumberValue("404")
fail "Error message"  // Expr = ASTStringValue("Error message")
```

**Test Example** (`ParserErrorHandlerTest.cpp`):
```cpp
void func() {
    fail
}
// ASTFailStmt with Expr = null
```

#### ASTHandleStmt
**Purpose**: Error handling block (catch/handle semantics).

**Structure**:
- **`ErrorHandler`**: `ASTExpr*` - typically `ASTIdentifier` for error variable
- **`Handle`**: `ASTBlockStmt*` - statements that may fail

**Syntax**: `['error' Identifier] 'handle' (Statement | Block)`

**Forms**:
1. **Simple handle**: `handle riskyOperation()`
2. **With block**: `handle { operations }`
3. **With error variable**: `error err handle { riskyOperation() }`

**Test Example** (`ParserErrorHandlerTest.cpp`):
```cpp
void func() {
    error err0 handle {
        fail
    }
}
// ASTHandleStmt:
//   ErrorHandler: ASTIdentifier("err0")
//   Handle: ASTBlockStmt containing fail statement
```

**Key Points**:
- Error variable is declared directly before `handle` keyword
- No assignment operator in handle syntax
- Error variable has type `error` (ASTBuiltinType::TYPE_ERROR)

### 3.6 Variables and Parameters

#### ASTVar Base
**Purpose**: Base class for all variable declarations.

**Location**: `include/AST/ASTVar.h`

**Structure**: Extends `ASTNode`

**Common Fields**:
- **`Type`**: `ASTType*` - declared type
- **`Name`**: `llvm::StringRef` - variable name
- **`Modifiers`**: `SmallVector<ASTModifier*, 8>` - visibility, const, static
- **`Expr`**: `ASTExpr*` - initializer expression (can be null)

**Note**: `ASTVar` is the abstract base class. Concrete implementations include:
- `ASTLocalVar` - Local variables within functions
- `ASTParam` - Function/method parameters

#### ASTLocalVar
**Purpose**: Local variable declarations within functions/blocks.

**Structure**: Inherits all fields from `ASTVar`

**Syntax**: `Type Identifier ['=' Expression]`

**Parser Behavior**:
1. Parser identifies local var when it sees a type followed by identifier (not in parameter list or class body)
2. Creates `ASTLocalVar` with type and name
3. Parses optional initializer after `=`
4. In newer design: wraps as `ASTExprStmt` containing assignment binary op

**Test Examples** (`ParserLocalVarTest.cpp`):
```cpp
void func() {
    bool a = false    // Type=bool, Name="a", Expr=ASTBoolValue(false)
    int e = 0         // Type=int, Name="e", Expr=ASTNumberValue("0")
    string s = "hi"   // Type=string, Name="s", Expr=ASTStringValue("hi")
    Type t = null     // Type=Type, Name="t", Expr=ASTNullValue
}
```

**Array Variables**:
```cpp
byte[] a              // Type=ASTArrayType(byte, null), Name="a"
byte[] b = {}         // With empty initializer
byte[] c = {1, 2, 3}  // With array literal
byte[3] d             // Fixed-size array
```

**Character Arrays**:
```cpp
byte[] c = {'a', 'b', 'c', ''}
// Expr = ASTArrayValue with 4 character values
```

**String Variables**:
```cpp
string a = ""         // Empty string
string b = "test"     // Regular string
```

#### ASTParam
**Purpose**: Function and method parameters.

**Structure**: Inherits from `ASTVar`

**Additional Fields**:
- **`Index`**: Parameter position (0-based)

**Syntax**: `Type Identifier`

**Example**:
```cpp
void func(int a, string b, bool c) { }
// Parameters:
//   [0]: Type=int, Name="a"
//   [1]: Type=string, Name="b"
//   [2]: Type=bool, Name="c"
```

**Key Point**: Parameters cannot have initializers in Fly.

#### ASTAttribute
**Purpose**: Class/struct field declarations.

**Structure**: Inherits from `ASTVar`

**Modifiers**:
- `public`, `private`, `protected` - visibility
- `const` - immutable field
- `static` - class-level field

**Test Example** (`ParserClassTest.cpp - Struct test`):
```cpp
public struct Test {
    int a                // No modifiers, no initializer
    public int b = 2     // Public with initializer
    const int c = 0      // Constant with initializer
}
// Attributes:
//   [0]: Name="a", Type=int, Modifiers=[], Expr=null
//   [1]: Name="b", Type=int, Modifiers=[public], Expr=2
//   [2]: Name="c", Type=int, Modifiers=[const], Expr=0
```

#### ASTEnumEntry
**Purpose**: Enum constant declarations.

**Structure**: Extends `ASTVar`

**Additional Fields**:
- **`Enum`**: `ASTEnum*` - reference to the parent enum
- **`Index`**: `uint32_t` - the numeric value/index of this entry (1-based)
- **`Comment`**: `llvm::StringRef` - documentation comment

**Syntax**: Identifier in space-separated list (note: not comma-separated)

**Example**:
```cpp
public enum Status { IDLE, RUNNING, STOPPED }
// Entries:
//   [0]: Name="IDLE"
//   [1]: Name="RUNNING"
//   [2]: Name="STOPPED"
```

**Test Example** (`ParserClassTest.cpp - Enum test`):
```cpp
public enum Test { A, B, C }
// Enum with 3 entries, comma-separated
```

### 3.7 Functions and Methods

#### ASTFunction
**Purpose**: Free function (top-level) declarations.

**Location**: `include/AST/ASTFunction.h`

**Structure**:
- **`Name`**: `llvm::StringRef` - function name
- **`ReturnType`**: `ASTType*` - return type
- **`Modifiers`**: `SmallVector<ASTModifier*, 8>` - Visibility and other modifiers
- **`Params`**: `SmallVector<ASTParam*, 8>` - parameter list
- **`Body`**: `ASTBlockStmt*` - function body (null for declarations)
- **`FunctionKind`**: `ASTFunctionKind` - distinguishes between functions and methods
  - `F_FUNCTION` - Regular function
  - `F_METHOD` - Method (used by `ASTMethod` subclass)
- **`Comment`**: `ASTComment*` - documentation comment

**Syntax**:
```
[Modifiers] ReturnType Identifier '(' [Parameters] ')' [Body]
```

**Test Example** (`ParserFunctionTest.cpp`):
```cpp
void func() {
    // body
}
// ASTFunction:
//   Name = "func"
//   ReturnType = ASTBuiltinType(TYPE_VOID)
//   Params = []
//   Body = ASTBlockStmt(...)
```

**With Parameters**:
```cpp
int add(int a, int b) {
    return a + b
}
// ASTFunction:
//   Name = "add"
//   ReturnType = ASTBuiltinType(TYPE_INT)
//   Params = [ASTParam("a", int), ASTParam("b", int)]
//   Body = ASTBlockStmt with return statement
```

#### ASTMethod
**Purpose**: Class/struct member functions.

**Location**: `include/AST/ASTMethod.h`

**Structure**: Extends `ASTFunction` (inherits all fields from `ASTFunction` and sets `FunctionKind` to `F_METHOD`)

**Method Modifiers**:
- `public`, `private`, `protected` - visibility
- `const` - method doesn't modify instance
- `static` - class method (no `this`)

**Test Example** (`ParserClassTest.cpp - Class test`):
```cpp
public class Test {
    public int a() { return a }
    protected int b() { return 2 }
    private int c() { return 3 }
    const int d() { return 0 }
}
// Methods:
//   [0]: Name="a", Modifiers=[public], ReturnType=int
//   [1]: Name="b", Modifiers=[protected], ReturnType=int
//   [2]: Name="c", Modifiers=[private], ReturnType=int
//   [3]: Name="d", Modifiers=[const], ReturnType=int
```

### 3.8 Classes and Structs

#### ASTClass
**Purpose**: Class, struct, and interface declarations.

**Location**: `include/AST/ASTClass.h`

**Structure**:
- **`Name`**: `llvm::StringRef` - class name
- **`ClassKind`**: `ASTClassKind`
  - `CLASS` - Reference type with methods
  - `INTERFACE` - Abstract interface
  - `STRUCT` - Value type
- **`Modifiers`**: `SmallVector<ASTModifier*, 8>` - Visibility and other modifiers
- **`Bases`**: `SmallVector<ASTType*, 4>` - base classes/interfaces (**comma-separated**)
- **`Nodes`**: `SmallVector<ASTNode*, 8>` - members (attributes, methods, constructors)
- **`Comment`**: `ASTComment*` - documentation

**Syntax**:
```
[Modifiers] ('class'|'struct'|'interface') Identifier [':' Base (',' Base)*] '{' [Members] '}'
```

**Base Class Syntax**: **CRITICAL - Comma-separated**
```cpp
public class Test : Class, Struct, Interface { }
// Bases:
//   [0]: ASTNamedType("Class")
//   [1]: ASTNamedType("Struct")
//   [2]: ASTNamedType("Interface")
```

**Test Example** (`ParserClassTest.cpp - ClassExtendAll`):
```cpp
public class Test : Class, Struct, Interface {}
// Parser consumes commas between base types
```

**Struct Example** (`ParserClassTest.cpp - Struct test`):
```cpp
public struct Test {
    int a
    public int b = 2
    const int c = 0
}
// ASTClass:
//   Name = "Test"
//   ClassKind = STRUCT
//   Modifiers = [public]
//   Nodes[0] = ASTAttribute("a", int)
//   Nodes[1] = ASTAttribute("b", int, public, init=2)
//   Nodes[2] = ASTAttribute("c", int, const, init=0)
```

**Class with Methods**:
```cpp
public class Test {
    int a = 1
    private int b = 1
    public int a() { return a }
    protected int b() { return 2 }
}
// ASTClass:
//   Name = "Test"
//   ClassKind = CLASS
//   Nodes[0] = ASTAttribute("a")
//   Nodes[1] = ASTAttribute("b", private)
//   Nodes[2] = ASTMethod("a", public)
//   Nodes[3] = ASTMethod("b", protected)
```

**Usage Example** (`ParserClassTest.cpp`):
```cpp
void func() {
    Test t = new Test()  // Constructor call
}
// ASTCall with CallKind = CALL_NEW
```

### 3.9 Enums

#### ASTEnum
**Purpose**: Enumeration type declarations.

**Location**: `include/AST/ASTEnum.h`

**Structure**:
- **`Name`**: `llvm::StringRef` - enum name
- **`Modifiers`**: `SmallVector<ASTModifier*, 8>` - Visibility modifiers
- **`Bases`**: `SmallVector<ASTType*, 4>` - optional base types/interfaces (comma-separated)
- **`Nodes`**: `SmallVector<ASTNode*, 8>` - contains enum entries and other declarations
- **`Comment`**: `ASTComment*`

**Note**: Enum values are stored internally in a `StringMap<ASTEnumEntry*> Vars` field, not directly in `Nodes`.

**Syntax**:
```
[Modifiers] 'enum' Identifier [':' Base (',' Base)*] '{' Entry [Entry]* '}'
```

**CRITICAL**: Enum entries are **space-separated**, not comma-separated.

**Test Example** (`ParserClassTest.cpp - Enum test`):
```cpp
public enum Test {
  A B C
}
// ASTEnum:
//   Name = "Test"
//   Modifiers = [public]
//   Vars["A"] = ASTEnumEntry("A", Index=1)
//   Vars["B"] = ASTEnumEntry("B", Index=2)
//   Vars["C"] = ASTEnumEntry("C", Index=3)
```

**Usage Example**:
```cpp
void main() {
    Test a = Test.A   // Member access to enum entry
    a = Test.B        // Assignment of enum value
    Test c = a        // Variable-to-variable assignment
}
// Accessing enum entries uses ASTIdentifier expressions
```

**With Base Types** (theoretical):
```cpp
public enum Status : BaseEnum, Interface {
    IDLE RUNNING STOPPED
}
// Bases parsed comma-separated like classes, entries space-separated
```

**Resolution**:
1. Parser creates `ASTEnum` with name and modifiers
2. Parses space-separated entry list
3. Each entry becomes `ASTEnumEntry` stored in the `Vars` StringMap
4. Resolver creates semantic representations for the enum type and entries
5. Entries are indexed starting from 1

### 3.10 Modifiers

#### ASTModifier
**Purpose**: Encodes visibility, mutability, and other declaration modifiers.

**Location**: `include/AST/ASTModifier.h`

**Modifier Kinds** (`ASTModifierKind`):
- **`MOD_PUBLIC`** - Public visibility
- **`MOD_PRIVATE`** - Private visibility
- **`MOD_PROTECTED`** - Protected visibility
- **`MOD_CONSTANT`** - Immutable (const)
- **`MOD_STATIC`** - Static/class-level

**Structure**:
- **`Kind`**: `ASTModifierKind`
- **`Location`**: Source location

**Usage**: Stored in `SmallVector<ASTModifier*, 4>` on:
- Functions
- Classes/Enums
- Attributes
- Methods

**Test Example**:
```cpp
public class Test {
    private int field
    public static void method() {}
}
// Class Modifiers: [MOD_PUBLIC]
// field Modifiers: [MOD_PRIVATE]
// method Modifiers: [MOD_PUBLIC, MOD_STATIC]
```

### 3.11 Comments

#### ASTComment
**Purpose**: Captures documentation comments for code generation and tools.

**Location**: `include/AST/ASTComment.h`

**Structure**:
- **`Content`**: `llvm::StringRef` - comment text
- **`IsBlock`**: `bool` - true for `/* */`, false for `//`

**Usage**: Attached to:
- Functions
- Classes
- Methods
- Attributes

**Syntax**:
```cpp
// Line comment
/* Block comment */
```

## 4. AST Construction Examples

### Example 1: Complete Function with Local Variables
**Source**:
```cpp
void calculate(int a, int b) {
    int sum = a + b
    int product = a * b
    return sum
}
```

**AST Structure**:
```
ASTFunction
  ├─ Name: "calculate"
  ├─ ReturnType: ASTBuiltinType(TYPE_VOID)
  ├─ Params:
  │   ├─ [0]: ASTParam(Name="a", Type=int)
  │   └─ [1]: ASTParam(Name="b", Type=int)
  └─ Body: ASTBlockStmt
      ├─ Content[0]: ASTExprStmt
      │   └─ Expr: ASTBinary(OP_BINARY_ASSIGN)
      │       ├─ Left: ASTIdentifier("sum")
      │       └─ Right: ASTBinary(OP_BINARY_ARITH_ADD)
      │           ├─ Left: ASTIdentifier("a")
      │           └─ Right: ASTIdentifier("b")
      ├─ Content[1]: ASTExprStmt
      │   └─ Expr: ASTBinary(OP_BINARY_ASSIGN)
      │       ├─ Left: ASTIdentifier("product")
      │       └─ Right: ASTBinary(OP_BINARY_ARITH_MUL)
      │           ├─ Left: ASTIdentifier("a")
      │           └─ Right: ASTIdentifier("b")
      └─ Content[2]: ASTReturnStmt
          └─ Expr: ASTIdentifier("sum")
```

### Example 2: Class with Inheritance and Methods
**Source**:
```cpp
public class MyClass : BaseClass, Interface {
    int value = 0
    public void setValue(int v) {
        value = v
    }
}
```

**AST Structure**:
```
ASTClass
  ├─ Name: "MyClass"
  ├─ ClassKind: CLASS
  ├─ Modifiers: [MOD_PUBLIC]
  ├─ Bases:
  │   ├─ [0]: ASTNamedType("BaseClass")
  │   └─ [1]: ASTNamedType("Interface")
  └─ Nodes:
      ├─ [0]: ASTAttribute
      │   ├─ Name: "value"
      │   ├─ Type: ASTBuiltinType(TYPE_INT)
      │   └─ Expr: ASTNumberValue("0")
      └─ [1]: ASTMethod
          ├─ Name: "setValue"
          ├─ Modifiers: [MOD_PUBLIC]
          ├─ ReturnType: ASTBuiltinType(TYPE_VOID)
          ├─ Params: [ASTParam("v", int)]
          └─ Body: ASTBlockStmt
              └─ Content[0]: ASTExprStmt
                  └─ Expr: ASTBinary(OP_BINARY_ASSIGN)
                      ├─ Left: ASTIdentifier("value")
                      └─ Right: ASTIdentifier("v")
```

### Example 3: Control Flow
**Source**:
```cpp
void process(int value) {
    if (value > 10) {
        return
    } elsif (value > 5) {
        value = value * 2
    } else {
        value = 0
    }
}
```

**AST Structure**:
```
ASTFunction("process")
  └─ Body: ASTBlockStmt
      └─ Content[0]: ASTIfStmt
          ├─ Rule: ASTBinary(OP_BINARY_COMPARE_GT)
          │   ├─ Left: ASTIdentifier("value")
          │   └─ Right: ASTNumberValue("10")
          ├─ Stmt: ASTBlockStmt
          │   └─ Content[0]: ASTReturnStmt(Expr=null)
          ├─ Elsif[0]: ASTRuleStmt
          │   ├─ Rule: ASTBinary(OP_BINARY_COMPARE_GT)
          │   │   ├─ Left: ASTIdentifier("value")
          │   │   └─ Right: ASTNumberValue("5")
          │   └─ Stmt: ASTBlockStmt
          │       └─ Content[0]: ASTExprStmt
          │           └─ Expr: ASTBinary(OP_BINARY_ASSIGN)
          │               ├─ Left: ASTIdentifier("value")
          │               └─ Right: ASTBinary(OP_BINARY_ARITH_MUL)
          └─ Else: ASTBlockStmt
              └─ Content[0]: ASTExprStmt
                  └─ Expr: ASTBinary(OP_BINARY_ASSIGN)
                      ├─ Left: ASTIdentifier("value")
                      └─ Right: ASTNumberValue("0")
```

## 5. Sema Fundamentals
| Component | Location | Notes |
|-----------|----------|-------|
| `SemaNode` | `include/Sema/SemaNode.h` | Base class with `SemaKind` enumeration (module, namespace, import, type, var, call, op, function, class, attribute, method, enum, enum entry, value).
| `SemaExpr` | `include/Sema/SemaExpr.h` | Adds parent/child expression linkage and cached `SemaType*`.
| `SemaFunctionBase` | `include/Sema/SemaFunctionBase.h` | Common state for free functions and class methods: mangled name, params, locals, return type, error handler, `CodeGenFunctionBase*` placeholder.
| `SemaBuilder` | `include/Sema/SemaBuilder.h` | Factory for every Sema subtype (functions, classes, vars, literals, calls, etc.).
| `Resolver` | `include/Sema/Resolver.h`, `src/Sema/Resolver.cpp` | Main visitor bridging AST to Sema. Manages scopes, modules, name spaces, classes/enums, and expressions.
| `Registry` | `include/Sema/Registry.h` | Tracks modules, namespaces, and built-in scopes used by the resolver.

## 6. Sema Types (Detailed)
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

## 7. Symbol Table & Scope Model
- `SymbolTable` (`include/Sema/SymbolTable.h`) implements a scoped lookup chain backed by `llvm::StringMap<Symbol*>`. `pushScope()` creates a child table linked via `Parent`.
- `Symbol` (`include/Sema/Symbol.h`) bundles a `Name`, `SemaKind`, and pointer to the referenced `SemaNode`.
- Resolver maintains `CurrentScope` and manipulates scopes via `EnterScope`/`ExitScope`. Local scopes correspond to blocks/functions/classes; module and namespace scopes come from the registry or class/enum symbol tables.
- Symbols are inserted immediately after SemaBuilder creates the corresponding semantic object (e.g., a function symbol inserted in the parent scope before resolving the body).

## 8. Sema Variables (Detailed)
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

## 9. Assignment Statement Resolution
Assignment statements in Fly follow a specific AST structure that distinguishes between the assignment operator `=` and other operators like equality `==`.

### Worked Example: Simple Assignment
**Source Code:**
```fly
void func(int a) {
    a = a + 1
}
```

**AST Structure:**
1. Parser creates `ASTExprStmt` containing an `ASTBinary(OP_BINARY_ASSIGN)`:
   - `left`: `ASTIdentifier("a")` - l-value
   - `right`: `ASTBinary(OP_BINARY_ARITH_ADD)` - r-value expression
     - `left`: `ASTIdentifier("a")`
     - `right`: `ASTNumberValue("1")`

**Resolution Steps:**
1. Resolver visits `ASTExprStmt` and its `ASTBinary(OP_BINARY_ASSIGN)`.
2. Left operand resolves to `SemaLocalVar` for parameter `a` (type: `SemaIntType`).
3. Right operand resolves recursively:
   - `OP_BINARY_ARITH_ADD` creates a `SemaBinary`.
   - Left: `SemaLocalVar` (a)
   - Right: `SemaIntValue(1)`
   - Result type: `SemaIntType`
4. Assignment operation validates that left-side type matches right-side type.

### Worked Example: Assignment with Equality Comparison
**Source Code:**
```fly
void func(bool result, int a) {
    result = a == 5
}
```

**AST Structure:**
1. Parser creates `ASTExprStmt` containing an `ASTBinary(OP_BINARY_ASSIGN)`:
   - `left`: `ASTIdentifier("result")`
   - `right`: `ASTBinary(OP_BINARY_COMPARE_EQ)` - **equality comparison**
     - `left`: `ASTIdentifier("a")`
     - `right`: `ASTNumberValue("5")`

**Key Points:**
- The outer `OP_BINARY_ASSIGN` handles the assignment operation.
- The inner `OP_BINARY_COMPARE_EQ` handles the equality comparison, which evaluates to boolean.
- Type checking ensures `result` (bool) can receive the result of `a == 5` (bool).
- **Never confuse** `OP_BINARY_ASSIGN` (assignment `=`) with `OP_BINARY_COMPARE_EQ` (equality `==`).

### Assignment Operator Kinds
| Operator | AST Kind | Semantic Meaning |
|----------|----------|------------------|
| `=` | `OP_BINARY_ASSIGN` | Assignment: stores right value into left l-value |
| `==` | `OP_BINARY_COMPARE_EQ` | Equality comparison: returns boolean |
| `!=` | `OP_BINARY_COMPARE_NE` | Inequality comparison: returns boolean |
| `+=` | `OP_BINARY_ASSIGN_ADD` | Compound assignment: `a += b` → `a = a + b` |
| `-=` | `OP_BINARY_ASSIGN_SUB` | Compound assignment: `a -= b` → `a = a - b` |

(Similar compound patterns for `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`, `>>=`)

## 10. Member Access Resolution
1. Parser produces `ASTMember` nodes (holding name, parent expression pointer, `ASTVar*` for the definition).
2. Resolver evaluates the parent expression to obtain a `SemaExpr*`. If the parent is a namespace, class, enum, or call, it delegates to the correct `ResolveChild` overload.
3. `ResolveChildMember` locates the target attribute via `SemaClassType::LookupAttribute`. It instantiates a `SemaMemberVar` via `SemaBuilder::CreateMemberVar`, linking the AST member, parent expression, and referenced `SemaClassAttribute`.
4. The resulting `SemaMemberVar` inherits the attribute's type, visibility, constant flag, and inserted symbol table entry (typically within the class scope when the attribute is declared, and within the current expression scope for temporary references).

## 11. Calls & Function Binding
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

## 12. Expressions & SemaExpr Chain
Every expression node (`ASTExpr`) holds a pointer to its semantic counterpart (`SemaExpr` subclasses such as `SemaVar`, `SemaCall`, `SemaValue`). Resolver ensures parent/child relationships are mirrored in the semantic tree, enabling later rewrites and diagnostics.

## 13. Functions, Classes, Enums in Sema
- **Functions**: `SemaFunction` couples an `ASTFunction` with its module, symbol table, comment, visibility, and `CodeGenFunction*`. Parameters and locals are added through `addParam`/`addLocalVar` as the resolver visits declarations.
- **Class Methods**: `SemaClassMethod` extends `SemaFunctionBase` with owning class, `this` instance, visibility, static flag, overridden method, comment, and `CodeGenClassMethod*`.
- **Classes**: `SemaClassType` (see §5), plus `SemaClassAttribute` for fields and `SemaClassInstance` for `this`.
- **Enums**: `SemaEnumType` with entry nodes (`SemaEnumEntry`). 
  - **Syntax**: `[Modifiers] 'enum' Identifier [':' BaseType] '{' Entry (',' Entry)* '}'`
  - **Example**: `public enum Status { IDLE, RUNNING, STOPPED }` creates an `ASTEnum` with three `ASTEnumEntry` children.
  - **Resolution**: Parser produces comma-separated `ASTEnumEntry` nodes. Resolver creates `SemaEnumType`, inserts it into the parent scope, then adds each `SemaEnumEntry` (with index, name, and `CodeGenEnumEntry*` slot) to the enum's symbol table.
  - **Member Access**: `Status.IDLE` is parsed as `ASTMember(parent=ASTIdentifier("Status"), name="IDLE")`. Resolver looks up `Status` → `SemaEnumType`, then finds `IDLE` in the enum's entry map, returning a `SemaEnumEntry`.
  - **Base Types**: Optional `: BaseType` allows enums to extend other enums or interfaces. Resolver validates the base chain and populates `SemaEnumType::getBases()`.

## 14. Error Handling with fail/handle

Fly implements error handling through `fail` and `handle` keywords, distinct from traditional exception mechanisms.

### ASTFailStmt
`ASTFailStmt` represents the `fail` keyword, which terminates function execution and propagates an error. It contains:
- **`Expr`**: Optional `ASTExpr*` representing the error payload (can be void, integer, string, or object).

**Syntax**: `fail [Expression]`

**Examples**:
- `fail` - Void failure (no payload)
- `fail 404` - Integer error code
- `fail "Error message"` - String error message

The parser makes the expression optional by checking if the next token is a statement terminator (closing brace, EOF, or another statement keyword) before calling `ParseExpr()`.

### ASTHandleStmt
`ASTHandleStmt` represents the `handle` keyword, which catches errors from enclosed statements. It contains:
- **`ErrorHandler`**: Optional `ASTExpr*` (typically `ASTIdentifier`) referencing the error variable declaration.
- **`Handle`**: `ASTBlockStmt*` containing the code to execute that may throw errors.

**Syntax**: `['error' Identifier] 'handle' (Statement | Block)`

**Forms**:
1. **Simple handle** (no error capture): `handle riskyOperation()`
2. **Handle with block**: `handle { operation1(); operation2(); }`
3. **Handle with error variable**: `error err handle { riskyOperation() }`

**Key Changes from Previous Design**:
- The assignment operator (`=`) has been removed from the handle syntax.
- Error variables are declared directly before `handle` without assignment.
- The `error` keyword creates a variable of type `error` that captures exception information.

**Parsing Flow**:
1. Parser detects `error` keyword in `ParseStmt` → recognizes it as a type via `isVarDecl`.
2. Creates `ASTLocalVar` with type `error` and the specified identifier.
3. Checks if next token is `handle` keyword → calls `ParseHandleStmt` with the identifier.
4. `ParseHandleStmt` consumes `handle`, parses the statement/block, and creates `ASTHandleStmt` with the identifier as `ErrorHandler`.

**Resolution**:
- The error variable becomes a `SemaLocalVar` with type `SemaErrorType`.
- `SemaErrorHandler` tracks the relationship between the call and the error variable.
- The handle block's statements are resolved within a nested scope.

## 15. Scope & Symbol Resolution Flow
1. **Module Entry**: Resolver creates a new `SemaModule`, registers it, sets namespace and scope, and visits top-level nodes in order. Namespaces must appear first.
2. **Imports**: `SemaImport` nodes are created. Symbols for imported namespaces are not immediately inserted; instead, resolver stores symbol tables for deferred lookup.
3. **Global Vars / Functions / Classes / Enums**: For each declaration, resolver creates the Sema object, inserts a symbol in the parent scope, then resolves nested content (e.g., function body, class members) under a pushed scope.
4. **Statements & Expressions**: Blocks call `EnterScope`/`ExitScope`. Locals are inserted into block-level symbol tables before resolving their initializers. Expressions delegate to `ResolveExpr`, `ResolveParent`, and `ResolveChild*` helpers.
5. **Errors**: Diagnostics are emitted through `Diag`. When a binding fails, resolver attaches `SemaErrorType` or `SemaVarKind::ERROR_VAR` placeholders so the pass can continue.

## 16. Worked Example: Local Variable Reference
1. Parser builds `ASTLocalVar` for `let x: int = 1;` and later an `ASTIdentifier` for `x`.
2. Resolver visits the declaration: creates `SemaLocalVar`, sets its `SemaType` to `SemaBuiltin::getIntType()`, inserts a `Symbol` `{ Name: "x", Kind: VAR, Ref: SemaLocalVar* }` into the block’s `SymbolTable`.
3. When resolving the identifier, `ResolveParent(ASTIdentifier*)` walks `CurrentScope` chain to find `x`, returning the `SemaLocalVar`. The identifier’s `SemaExpr*` becomes that `SemaVar`, and its `Type` pointer now references the underlying `SemaType`.

## 17. Complete AST Header Reference
The table below lists every header in `include/AST/` and the primary constructs it defines.

| Header | Key Types / Responsibilities | Notes |
|--------|------------------------------|-------|
| `ASTArg.h` | `ASTArg` | Represents positional call arguments with index bookkeeping. |
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
| `ASTEnum.h` | `ASTEnum` | Enum declarations with comma-separated entries, modifiers, and optional base types. |
| `ASTEnumEntry.h` | `ASTEnumEntry` | Individual enum entries tied to `SemaEnumEntry`. |
| `ASTExpr.h` | `ASTExpr`, `ASTExprKind` | Base for all expressions, tracks semantic attachments. |
| `ASTExprStmt.h` | `ASTExprStmt` | Statement wrapper holding a standalone expression (including assignments via `OP_BINARY_ASSIGN`). |
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
| `ASTOp.h` | `ASTUnary`, `ASTBinary`, `ASTTernary`, `Precedence` and enum variants | All operator expressions and precedence utilities. Assignments use `OP_BINARY_ASSIGN`. Note: These are found in separate headers (`ASTUnary.h`, `ASTBinary.h`, `ASTTernary.h`). |
| `ASTParam.h` | `ASTParam` | Function/method parameter declaration nodes. |
| `ASTReturnStmt.h` | `ASTReturnStmt` | `return` statement AST. |
| `ASTRuleStmt.h` | `ASTRuleStmt` | Rule-based statement used by pattern/DSL features. |
| `ASTStmt.h` | `ASTStmt`, `ASTStmtKind` | Base for statements with parent/function tracking. |
| `ASTSwitchStmt.h` | `ASTSwitchStmt`, helper block types | `switch`/`case` representation. |
| `ASTType.h` | `ASTType`, `ASTBuiltinType`, `ASTNamedType`, `ASTArrayType`, enums | Type syntax nodes and built-in identifiers. |
| `ASTValue.h` | `ASTValue` hierarchy (`ASTBoolValue`, `ASTNumberValue`, ... ) | Literal expressions and aggregate literal forms. |
| `ASTVar.h` | `ASTVar`, `ASTVarKind` | Base for all variable declarations. |
| `ASTVisitor.h` | `ASTVisitor` | Visitor interface used by resolver and other passes. |

## 18. Complete Sema Header Reference
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
