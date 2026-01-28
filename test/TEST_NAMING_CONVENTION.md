# CodeGen Test Naming Convention

## Date: January 28, 2026

## Objective
Establish a clear and consistent naming convention for AST objects in CodeGen tests that makes the relationship between C++ test code and Fly source code immediately obvious.

## Naming Convention Rules

### 1. **ASTLocalVar** - Local Variables
**Pattern:** `LocalVar_<name>`

**Example:**
```cpp
// Fly code: int a
ASTLocalVar *LocalVar_a = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "a", EmptyModifiers);
```

### 2. **ASTParam** - Function Parameters  
**Pattern:** `Param_<name>`

**Example:**
```cpp
// Fly code: void func(int a)
ASTParam *Param_a = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
```

### 3. **ASTIdentifier** - Variable References
**Pattern:** `Identifier_<name>[_context]`

**Example:**
```cpp
// Fly code: a (reference to variable 'a')
ASTIdentifier *Identifier_a = ASTBuilder::CreateIdentifier(LocalVar_a);

// With context when multiple references exist:
ASTIdentifier *Identifier_aExpr = ASTBuilder::CreateIdentifier(Param_a);
ASTIdentifier *Identifier_iInit = ASTBuilder::CreateIdentifier(LocalVar_i);  // in loop init
ASTIdentifier *Identifier_iCond = ASTBuilder::CreateIdentifier(LocalVar_i);  // in loop condition
ASTIdentifier *Identifier_iPost = ASTBuilder::CreateIdentifier(LocalVar_i);  // in loop post
```

### 4. **ASTDeclStmt** - Declaration Statements
**Pattern:** `DeclStmt_<name>`

**Example:**
```cpp
// Fly code: int a
ASTDeclStmt *DeclStmt_a = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_a);
```

### 5. **ASTExprStmt** - Expression Statements
**Pattern:** `ExprStmt_<name>[_operation]`

**Example:**
```cpp
// Fly code: a = 0
ASTExprStmt *ExprStmt_a = ASTBuilder::CreateExprStmt(Body, SourceLoc);

// With operation context:
ASTExprStmt *ExprStmt_cEq = ASTBuilder::CreateExprStmt(Body, SourceLoc);  // c = (comparison)
ASTExprStmt *ExprStmt_a1 = ASTBuilder::CreateExprStmt(IfBlock, SourceLoc);  // a = 1 in if
ASTExprStmt *ExprStmt_a2 = ASTBuilder::CreateExprStmt(ElseBlock, SourceLoc);  // a = 2 in else
```

### 6. **ASTNumberValue** - Numeric Literals
**Pattern:** `NumberValue_<var><value>` or `NumberValue_<value>`

**Example:**
```cpp
// Fly code: a = 1
ASTNumberValue *NumberValue_a1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");

// Generic number:
ASTNumberValue *NumberValue_0 = ASTBuilder::CreateNumberValue(SourceLoc, "0");
ASTNumberValue *NumberValue_1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
```

### 7. **ASTBoolValue** - Boolean Literals
**Pattern:** `BoolValue_<var><value>` or `BoolValue_<true|false>`

**Example:**
```cpp
// Fly code: a = false
ASTBoolValue *BoolValue_aFalse = ASTBuilder::CreateBoolValue(SourceLoc, false);
```

### 8. **ASTBinary** - Binary Expressions
**Pattern:** `BinaryExpr_<operation>` or `AssignExpr_<name>[_operation]`

**Example:**
```cpp
// Fly code: a = b
ASTBinary *AssignExpr_a = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ...);

// Fly code: c = a == b
ASTBinary *BinaryExpr_cEq = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ, ...);
ASTBinary *AssignExpr_cEq = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ...);

// Fly code: c = a || b  
ASTBinary *BinaryExpr_cOr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_LOGIC_OR, ...);
ASTBinary *AssignExpr_cOr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ...);
```

### 9. **ASTTernary** - Ternary Expressions
**Pattern:** `TernaryExpr_<name>`

**Example:**
```cpp
// Fly code: c = a == b ? a : b
ASTTernary *TernaryExpr_c = ASTBuilder::CreateTernary(CondExpr_c, ...);
```

### 10. **Condition Expressions**
**Pattern:** `CondExpr_<name>` or `IfCond`

**Example:**
```cpp
// Fly code: if (a == b)
ASTBinary *CondExpr_c = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ, ...);

// Generic if condition:
ASTBinary *IfCond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ, ...);
```

## Handling Duplicates

When the same variable is used in multiple contexts, add a **disambiguating suffix**:

### By Number (when same operation repeated)
```cpp
// Fly code:
//   int a = 0  // First assignment
//   a = 1      // Second assignment
//   a = 2      // Third assignment

ASTNumberValue *NumberValue_a0 = ASTBuilder::CreateNumberValue(SourceLoc, "0");
ASTBinary *AssignExpr_a = ASTBuilder::CreateBinary(..., NumberValue_a0);

ASTNumberValue *NumberValue_a1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
ASTBinary *AssignExpr_a1 = ASTBuilder::CreateBinary(..., NumberValue_a1);

ASTNumberValue *NumberValue_a2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
ASTBinary *AssignExpr_a2 = ASTBuilder::CreateBinary(..., NumberValue_a2);
```

### By Operation (when different operations)
```cpp
// Fly code:
//   c = a == b  // Equality comparison
//   c = a != b  // Inequality comparison
//   c = a > b   // Greater than comparison

ASTExprStmt *ExprStmt_cEq = ...;   // c = (equality)
ASTExprStmt *ExprStmt_cNeq = ...;  // c = (not equal)
ASTExprStmt *ExprStmt_cGt = ...;   // c = (greater than)
```

### By Context (when used in different blocks)
```cpp
// Fly code:
//   if (a == 1) {
//       a = 2
//   } else {
//       a = 3
//   }

ASTExprStmt *ExprStmt_a2 = ASTBuilder::CreateExprStmt(IfBlock, ...);    // in if block
ASTExprStmt *ExprStmt_a3 = ASTBuilder::CreateExprStmt(ElseBlock, ...);  // in else block
```

## Complete Example

### Fly Code:
```fly
void func(int a) {
    int b
    bool c
    
    b = 0
    c = a == b
}
```

### C++ Test Code (Following Convention):
```cpp
// func(int a)
llvm::SmallVector<ASTParam *, 8> Params;
ASTParam *Param_a = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
Params.push_back(Param_a);
ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

// int b
ASTLocalVar *LocalVar_b = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "b", EmptyModifiers);
ASTDeclStmt *DeclStmt_b = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_b);

// bool c  
ASTLocalVar *LocalVar_c = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "c", EmptyModifiers);
ASTDeclStmt *DeclStmt_c = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_c);

// b = 0
ASTExprStmt *ExprStmt_b = ASTBuilder::CreateExprStmt(Body, SourceLoc);
ASTNumberValue *NumberValue_b0 = ASTBuilder::CreateNumberValue(SourceLoc, "0");
ASTBinary *AssignExpr_b = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, 
                                                   ASTBuilder::CreateIdentifier(LocalVar_b), 
                                                   NumberValue_b0);
ExprStmt_b->setExpr(AssignExpr_b);

// c = a == b
ASTExprStmt *ExprStmt_cEq = ASTBuilder::CreateExprStmt(Body, SourceLoc);
ASTBinary *BinaryExpr_cEq = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ,
                                                     ASTBuilder::CreateIdentifier(Param_a),
                                                     ASTBuilder::CreateIdentifier(LocalVar_b));
ASTBinary *AssignExpr_cEq = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                     ASTBuilder::CreateIdentifier(LocalVar_c),
                                                     BinaryExpr_cEq);
ExprStmt_cEq->setExpr(AssignExpr_cEq);
```

## Benefits

1. **✅ Clear Mapping** - Easy to see which C++ object corresponds to which Fly variable
2. **✅ Searchable** - Can grep for `LocalVar_a` to find all uses of variable 'a'
3. **✅ Type Obvious** - The prefix tells you the AST node type immediately
4. **✅ Consistent** - Same pattern applied throughout all tests
5. **✅ Self-Documenting** - Variable names explain their purpose
6. **✅ No Confusion** - Disambiguating suffixes handle duplicates clearly

## Files Updated

- `test/CodeGen/CodeGenBaseTest.cpp` - All variable naming updated to follow convention

## Statistics

- **Total Patterns Updated:** 50+
- **Variables Renamed:** 100+
- **Tests Updated:** 15+
- **Build Status:** ✅ Success

## Summary

The naming convention makes the relationship between the Fly source code (shown in comments) and the C++ AST construction code immediately clear. Each variable name consists of:

1. **AST Type Prefix** - What kind of node it is
2. **Fly Variable Name** - The name from the Fly code
3. **Context/Number Suffix** - (Optional) Disambiguation when needed

This pattern ensures that anyone reading the tests can instantly understand what AST is being built and how it maps to the Fly language code being tested.

