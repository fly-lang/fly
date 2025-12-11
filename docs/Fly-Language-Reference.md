# Fly Language Reference

**Version:** 1.0  
**Project:** [Fly Programming Language](https://flylang.org)  
**License:** Apache License v2.0

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Lexical Elements](#2-lexical-elements)
3. [Types](#3-types)
4. [Variables](#4-variables)
5. [Functions](#5-functions)
6. [Classes and Structures](#6-classes-and-structures)
7. [Enumerations](#7-enumerations)
8. [Expressions](#8-expressions)
9. [Statements](#9-statements)
10. [Namespaces and Imports](#10-namespaces-and-imports)
11. [Modifiers](#11-modifiers)
12. [Comments](#12-comments)
13. [Grammar Summary](#13-grammar-summary)

---

## 1. Introduction

Fly is a compiled, high-level, general-purpose programming language with particular attention to simplicity, readability, and multi-paradigm support. Fly is built on LLVM infrastructure and aims to provide optional Garbage Collection.

**Design Principles:**
- **Simple** - Easy to read and write
- **Fast** - Compiled with LLVM for optimal performance
- **Powerful** - Multi-paradigm with modern features

---

## 2. Lexical Elements

### 2.1 Keywords

Fly reserves the following keywords:

```
as          bool        break       byte        case
char        class       const       continue    default
double      elsif       else        enum        error
fail        false       float       for         handle
if          import      interface   int         long
namespace   new         null        private     protected
public      return      short       static      string
struct      switch      true        uint        ulong
ushort      void        while
```

### 2.2 Identifiers

Identifiers must start with a letter or underscore, followed by any combination of letters, digits, or underscores.

**Syntax:**
```
Identifier ::= [a-zA-Z_][a-zA-Z0-9_]*
```

**Examples:**
```fly
myVariable
_privateVar
counter123
MyClass
getValue
```

### 2.3 Literals

#### 2.3.1 Numeric Literals

```fly
42          // integer literal
0           // zero
3.14        // floating-point literal
0.0         // floating-point zero
```

#### 2.3.2 Boolean Literals

```fly
true        // boolean true
false       // boolean false
```

#### 2.3.3 Character Literals

```fly
'a'         // character
'Z'         // uppercase character
'\n'        // newline escape
```

#### 2.3.4 String Literals

```fly
"Hello, World!"
"Fly Language"
""          // empty string
```

#### 2.3.5 Null Literal

```fly
null        // null value for reference types
```

### 2.4 Operators and Punctuators

#### Arithmetic Operators
```fly
+           // addition
-           // subtraction
*           // multiplication
/           // division
%           // modulo
++          // increment
--          // decrement
```

#### Compound Assignment Operators
```fly
+=          // add and assign
-=          // subtract and assign
*=          // multiply and assign
/=          // divide and assign
%=          // modulo and assign
```

#### Comparison Operators
```fly
==          // equal to
!=          // not equal to
<           // less than
>           // greater than
<=          // less than or equal
>=          // greater than or equal
```

#### Logical Operators
```fly
&&          // logical AND
||          // logical OR
!           // logical NOT
```

#### Bitwise Operators
```fly
&           // bitwise AND
|           // bitwise OR
^           // bitwise XOR
<<          // left shift
>>          // right shift
&=          // bitwise AND and assign
|=          // bitwise OR and assign
^=          // bitwise XOR and assign
<<=         // left shift and assign
>>=         // right shift and assign
```

#### Other Operators and Punctuators
```fly
=           // assignment
?:          // ternary conditional
.           // member access
[]          // array subscript
()          // function call / grouping
{}          // block delimiters
,           // separator
;           // statement terminator (optional)
:           // label/case separator
...         // ellipsis
@           // annotation
```

---

## 3. Types

### 3.1 Built-in Types

#### 3.1.1 Integer Types

| Type     | Size    | Range                                    | Description              |
|----------|---------|------------------------------------------|--------------------------|
| `byte`   | 8-bit   | 0 to 255                                 | Unsigned byte            |
| `short`  | 16-bit  | -32,768 to 32,767                        | Signed short integer     |
| `ushort` | 16-bit  | 0 to 65,535                              | Unsigned short integer   |
| `int`    | 32-bit  | -2,147,483,648 to 2,147,483,647          | Signed integer           |
| `uint`   | 32-bit  | 0 to 4,294,967,295                       | Unsigned integer         |
| `long`   | 64-bit  | -9,223,372,036,854,775,808 to ...        | Signed long integer      |
| `ulong`  | 64-bit  | 0 to 18,446,744,073,709,551,615          | Unsigned long integer    |

**Examples:**
```fly
byte age = 25
short temperature = -10
ushort port = 8080
int count = 1000
uint id = 12345
long bigNum = 9999999999
ulong hugeNum = 18446744073709551615
```

#### 3.1.2 Floating-Point Types

| Type     | Size    | Description                    |
|----------|---------|--------------------------------|
| `float`  | 32-bit  | Single-precision float         |
| `double` | 64-bit  | Double-precision float         |

**Examples:**
```fly
float pi = 3.14
double precise = 3.14159265359
```

#### 3.1.3 Other Built-in Types

| Type     | Description                           |
|----------|---------------------------------------|
| `bool`   | Boolean type (true or false)          |
| `char`   | Character type                        |
| `string` | String type                           |
| `void`   | No type (for functions with no return)|
| `error`  | Error type for error handling         |

**Examples:**
```fly
bool isActive = true
char letter = 'A'
string name = "Fly"
```

### 3.2 Array Types

Arrays can be declared with or without explicit size.

**Syntax:**
```
ArrayType ::= Type '[' [ Expression ] ']'
```

**Examples:**
```fly
// Dynamic array (size unspecified)
byte[] dynamicArray
int[] numbers

// Fixed-size array
byte[10] fixedBuffer
int[5] coordinates

// Multi-dimensional arrays
int[][] matrix
byte[][][] cube
```

### 3.3 Named Types

User-defined types include classes, structures, and enumerations.

**Examples:**
```fly
MyClass obj
Point location
Status currentStatus
```

### 3.4 Qualified Type Names

Types can be qualified with namespace prefixes.

**Examples:**
```fly
// Using dotted notation
utils.Helper helper
mylib.DataType data
```

---

## 4. Variables

### 4.1 Local Variables

Local variables are declared within functions or blocks.

**Syntax:**
```
LocalVar ::= [ Modifiers ] Type Identifier [ '=' Expression ]
```

**Examples:**
```fly
void func() {
    // Simple declaration
    int x = 10
    
    // Without initialization
    bool flag
    
    // Constant local variable
    const int limit = 100
}
```

### 4.2 Variable Initialization

#### 4.2.1 Basic Types

```fly
bool flag = true
int count = 42
float value = 3.14
string message = "Hello"
```

#### 4.2.2 Null Initialization

```fly
MyClass obj = null
Type instance = null
```

#### 4.2.3 Array Initialization

```fly
// Empty array
byte[] empty = {}

// Array with values
byte[] values = {1, 2, 3, 4, 5}
int[] numbers = {10, 20, 30}

// Fixed-size array
byte[3] buffer = {1, 2, 3}
```

---

## 5. Functions

### 5.1 Function Declaration

**Syntax:**
```
Function ::= [ Modifiers ] Type Identifier '(' [ Parameters ] ')' ( Block | ';' )
```

**Examples:**
```fly
// Simple function
void doSomething() {
    // function body
}

// Function with return type
int calculate() {
    return 42
}

// Function with parameters
int add(int a, int b) {
    return a + b
}
```

### 5.2 Function Parameters

**Syntax:**
```
Parameters ::= Parameter ( ',' Parameter )*
Parameter  ::= [ 'const' ] Type Identifier
```

**Examples:**
```fly
// Multiple parameters
void process(int x, float y, bool flag) {
    // implementation
}

// Constant parameters
void readOnly(const int size, const string name) {
    // size and name cannot be modified
}

// Array parameters
void processArray(byte[] data, int[] indices) {
    // implementation
}
```

### 5.3 Visibility Modifiers

Functions can have different visibility levels:

```fly
// Default visibility (package-private)
void defaultFunction() {}

// Private function (internal use only)
private void privateHelper() {}

// Public function (exported)
public void publicAPI() {}

// Protected function (for inheritance)
protected void protectedMethod() {}
```

### 5.4 Return Statement

```fly
// Return void (no value)
void noReturn() {
    return
}

// Return a value
int getValue() {
    return 42
}

// Return expression
int compute(int a, int b) {
    return a * b + 10
}
```

### 5.5 The Main Function

The `main()` function is the entry point of a Fly application.

**Syntax:**
```fly
void main() {
    // Application code
}
```

**Key Characteristics:**

1. **Function signature:** Must be declared as `void main()` with no parameters
2. **Entry point:** The application starts execution from `main()`
3. **Automatic error handling:** The main function has special error handling behavior

**Error Handling and Return Codes:**

When the application runs, `main()` automatically returns an exit code to the operating system:

- **Return 0:** If no unhandled errors occur (success)
- **Return 1:** If an unhandled error occurs (failure)

This behavior is automatic—you don't explicitly return an integer from `main()`.

**Example 1: Successful Execution**
```fly
void main() {
    // Code executes successfully
    int x = 10
    int y = 20
    // Automatically returns 0 (success)
}
```

**Example 2: Unhandled Error**
```fly
void err0() {
    fail "Something went wrong"
}

void main() {
    err0()  // Error is not handled
    // Automatically returns 1 (failure)
}
```

**Example 3: Handled Error**
```fly
void err0() {
    fail "Something went wrong"
}

void main() {
    handle err0()  // Error is caught and handled
    // Continues execution
    // Automatically returns 0 (success)
}
```

**Example 4: Captured Error with Graceful Handling**
```fly
void riskyOperation() {
    fail "Operation failed"
}

void main() {
    error err = handle {
        riskyOperation()
    }
    
    if (err) {
        // Error was caught and handled
        // Continue with fallback logic
    }
    // Automatically returns 0 (success)
}
```

**Best Practices:**

1. **Always handle errors in main:** Unhandled errors will cause the application to exit with code 1
2. **Use handle blocks:** Wrap risky operations in `handle` blocks to ensure graceful error handling
3. **Check error variables:** Use `if (err)` to detect and respond to errors appropriately
4. **Provide fallback logic:** When errors occur, provide alternative execution paths

**Summary:**
- `void main()` is required (not `int main()`)
- Exit code 0 = success (no unhandled errors)
- Exit code 1 = failure (unhandled error occurred)
- Use `handle` to catch errors and ensure successful exit

---

## 6. Classes and Structures

### 6.1 Class Declaration

**Syntax:**
```
Class ::= [ Modifiers ] 'class' Identifier [ ':' BaseList ] '{' ClassMember* '}'
```

**Examples:**
```fly
// Simple class
class MyClass {
}

// Public class
public class Application {
}

// Class with inheritance
class Derived : Base {
}

// Class with multiple bases
class MyClass : Base1, Base2 {
}
```

### 6.2 Structure Declaration

Structures are value types similar to classes.

**Syntax:**
```
Struct ::= [ Modifiers ] 'struct' Identifier [ ':' BaseList ] '{' StructMember* '}'
```

**Examples:**
```fly
// Simple structure
struct Point {
    int x
    int y
}

// Public structure
public struct Vector {
    float x
    float y
    float z
}
```

### 6.3 Interface Declaration

Interfaces define contracts for classes.

**Syntax:**
```
Interface ::= [ Modifiers ] 'interface' Identifier '{' InterfaceMember* '}'
```

**Examples:**
```fly
// Simple interface
interface IDrawable {
    void draw()
}

// Public interface
public interface ISerializable {
    string serialize()
    void deserialize(string data)
}
```

### 6.4 Class Members

Classes can contain fields (attributes) and methods.

**Examples:**
```fly
public class Person {
    // Private fields
    private string name
    private int age
    
    // Public field
    public bool isActive
    
    // Constructor-like method
    public void initialize(string personName, int personAge) {
        name = personName
        age = personAge
        isActive = true
    }
    
    // Public method
    public string getName() {
        return name
    }
    
    // Private method
    private void validate() {
        // validation logic
    }
    
    // Static field
    static int instanceCount = 0
    
    // Static method
    public static int getCount() {
        return instanceCount
    }
}
```

### 6.5 Object Creation

**Examples:**
```fly
// Create instance
MyClass obj = new MyClass()

// Use with initialization
Person person = new Person()
person.initialize("John", 30)
```

---

## 7. Enumerations

### 7.1 Enum Declaration

**Syntax:**
```
Enum ::= [ Modifiers ] 'enum' Identifier [ ':' BaseType ] '{' EnumEntryList '}'
EnumEntryList ::= EnumEntry ( ',' EnumEntry )*
EnumEntry ::= Identifier
```

**Examples:**
```fly
// Simple enum with comma-separated entries
enum Color {
    RED, GREEN, BLUE
}

// Public enum
public enum Status {
    IDLE, RUNNING, STOPPED, FAILED
}

// Enum extending a base type
public enum Option : Enum {
    A, B, C
}

// Multi-line enum for readability
enum Direction {
    NORTH,
    SOUTH,
    EAST,
    WEST
}
```

### 7.2 Using Enums

**Examples:**
```fly
void processColor() {
    // Declare and initialize
    Color c = Color.RED
    
    // Assignment
    c = Color.BLUE
    
    // Pass to function
    setColor(Color.GREEN)
    
    // Compare
    if (c == Color.RED) {
        // handle red
    }
}

void setColor(Color c) {
    // use color
}

// Example with enum extending base type
void testOption() {
    Option opt = Option.A
    opt = Option.B
}
```

---

## 8. Expressions

### 8.1 Primary Expressions

#### 8.1.1 Literals

```fly
42              // integer literal
3.14            // float literal
true            // boolean literal
'c'             // character literal
"string"        // string literal
null            // null literal
```

#### 8.1.2 Identifiers

```fly
myVariable      // simple identifier
obj.field       // member access
array[0]        // array access
```

#### 8.1.3 Parenthesized Expressions

```fly
(a + b)
(x * y + z)
```

### 8.2 Unary Expressions

**Syntax:**
```
UnaryExpr ::= ( '++' | '--' | '!' | '-' | '+' ) Expression
            | Expression ( '++' | '--' )
```

**Examples:**
```fly
// Pre-increment/decrement
++counter
--index

// Post-increment/decrement
value++
count--

// Logical negation
!flag
!isActive

// Unary minus/plus
-value
+number
```

### 8.3 Binary Expressions

#### 8.3.1 Arithmetic Operators

```fly
a + b           // addition
x - y           // subtraction
m * n           // multiplication
p / q           // division
r % s           // modulo
```

#### 8.3.2 Comparison Operators

```fly
a == b          // equal to
x != y          // not equal to
m < n           // less than
p > q           // greater than
i <= j          // less than or equal
k >= l          // greater than or equal
```

#### 8.3.3 Logical Operators

```fly
flag1 && flag2  // logical AND
cond1 || cond2  // logical OR
```

#### 8.3.4 Bitwise Operators

```fly
a & b           // bitwise AND
x | y           // bitwise OR
m ^ n           // bitwise XOR
p << 2          // left shift
q >> 1          // right shift
```

### 8.4 Assignment Expressions

**Syntax:**
```
Assignment ::= Identifier AssignOp Expression
AssignOp   ::= '=' | '+=' | '-=' | '*=' | '/=' | '%=' 
             | '&=' | '|=' | '^=' | '<<=' | '>>='
```

**Examples:**
```fly
// Simple assignment
x = 10
name = "Fly"

// Compound assignment
a += 5          // a = a + 5
b -= 3          // b = b - 3
c *= 2          // c = c * 2
d /= 4          // d = d / 4
e %= 7          // e = e % 7

// Bitwise compound assignment
f &= mask       // f = f & mask
g |= flag       // g = g | flag
h ^= toggle     // h = h ^ toggle
i <<= 2         // i = i << 2
j >>= 1         // j = j >> 1
```

#### 8.4.1 Assignment vs Equality: Important Distinction

Fly clearly distinguishes between the **assignment operator** `=` and the **equality comparison operator** `==`:

- **`=` (Assignment)**: Stores a value into a variable. This is a statement-level operation.
- **`==` (Equality)**: Compares two values for equality. This is an expression that evaluates to a boolean.

**Examples:**
```fly
// Assignment: stores the value 5 into variable x
x = 5

// Equality comparison: compares x with 5, evaluates to boolean
if (x == 5) {
    // x is equal to 5
}

// Assignment with equality comparison on right side
result = x == 5    // result gets true or false

// Complex example
a = a + 1          // a = (a + 1) - addition then assignment
b = a == 10        // b = (a == 10) - comparison then assignment
```

**Parser Representation:**
Under the hood, the parser creates different AST structures:
- Assignment `a = expr` creates `ASTBinaryOp(OP_BINARY_ASSIGN)` with left=`a` and right=`expr`
- Equality `a == b` creates `ASTBinaryOp(OP_BINARY_EQ)` with left=`a` and right=`b`
- Assignment with equality `a = (b == c)` creates nested structure:
  - Outer: `ASTBinaryOp(OP_BINARY_ASSIGN)` with left=`a`
  - Right child: `ASTBinaryOp(OP_BINARY_EQ)` with left=`b` and right=`c`

**Common Mistake:**
```fly
// WRONG: Using = instead of == in condition
if (x = 5) {        // This assigns 5 to x, then evaluates the result
    // ...
}

// CORRECT: Using == for comparison
if (x == 5) {       // This compares x with 5
    // ...
}
```

### 8.5 Ternary Conditional Expression

**Syntax:**
```
TernaryExpr ::= Condition '?' TrueExpr ':' FalseExpr
```

**Examples:**
```fly
result = condition ? valueIfTrue : valueIfFalse
max = a > b ? a : b
status = isActive ? Status.RUNNING : Status.IDLE
```

### 8.6 Function Call Expressions

**Examples:**
```fly
// Function call without arguments
result = calculate()

// Function call with arguments
sum = add(10, 20)
process(x, y, z)

// Method call
obj.doSomething()
person.getName()
```

### 8.7 Array Value Expressions

**Examples:**
```fly
// Empty array
empty = {}

// Array with values
values = {1, 2, 3, 4, 5}
matrix = {{1, 2}, {3, 4}}
```

---

## 9. Statements

### 9.1 Expression Statements

Any expression can be used as a statement.

**Examples:**
```fly
// Function call
doSomething()
calculate()

// Increment/decrement
counter++
--index

// Assignment
x = 42
```

### 9.2 Block Statements

**Syntax:**
```
Block ::= '{' Statement* '}'
```

**Examples:**
```fly
{
    int x = 10
    int y = 20
    int z = x + y
}
```

### 9.3 If Statements

**Syntax:**
```
IfStmt ::= 'if' [ '(' ] Expression [ ')' ] Statement
           ( 'elsif' [ '(' ] Expression [ ')' ] Statement )*
           [ 'else' Statement ]
```

**Examples:**
```fly
// Simple if
if (condition) {
    // code
}

// If without parentheses
if condition {
    // code
}

// If-else
if (x > 0) {
    positive = true
} else {
    positive = false
}

// If-elsif-else
if (a == 1) {
    b = 0
} elsif (a == 2) {
    b = 1
} elsif (a == 3) {
    b = 2
} else {
    b = -1
}

// Inline if (without braces)
if (condition) doSomething()
```

### 9.4 Switch Statements

**Syntax:**
```
SwitchStmt  ::= 'switch' [ '(' ] Expression [ ')' ] '{' CaseClause* [ DefaultClause ] '}'
CaseClause  ::= 'case' Expression ':' Statement*
DefaultClause ::= 'default' ':' Statement*
```

**Examples:**
```fly
switch (value) {
    case 1:
        // code for case 1
        break
    case 2:
        // code for case 2
        break
    case 3:
    case 4:
        // code for case 3 and 4 (fall-through)
        break
    default:
        // default code
}

// Without parentheses
switch value {
    case 0:
        result = "zero"
        break
    default:
        result = "other"
}
```

### 9.5 Loop Statements

#### 9.5.1 While Loop

**Syntax:**
```
WhileStmt ::= 'while' [ '(' ] Expression [ ')' ] Statement
```

**Examples:**
```fly
// While with parentheses
while (count < 10) {
    count++
}

// While without parentheses
while count < 10 {
    count++
}

// Infinite loop
while true {
    // loop body
    if (shouldBreak) break
}

// Inline while
while condition doSomething()
```

#### 9.5.2 For Loop

**Syntax:**
```
ForStmt ::= 'for' VarDecl ( ',' VarDecl )* ';' Expression ';' 
            Expression ( ',' Expression )* Statement
```

**Examples:**
```fly
// Standard for loop
for int i = 0; i < 10; i++ {
    // loop body
}

// Multiple initialization and post expressions
for int i = 0, int j = 10; i < j; i++, j-- {
    // loop body
}

// For loop without parentheses
for int i = 0; i < length; i++ {
    process(array[i])
}
```

### 9.6 Jump Statements

#### 9.6.1 Return Statement

**Syntax:**
```
ReturnStmt ::= 'return' [ Expression ]
```

**Examples:**
```fly
// Return without value
return

// Return with value
return 42

// Return expression
return a + b * c
```

#### 9.6.2 Break Statement

**Syntax:**
```
BreakStmt ::= 'break'
```

**Examples:**
```fly
while true {
    if (condition) {
        break  // exit loop
    }
}

switch (value) {
    case 1:
        doSomething()
        break  // exit switch
}
```

#### 9.6.3 Continue Statement

**Syntax:**
```
ContinueStmt ::= 'continue'
```

**Examples:**
```fly
for int i = 0; i < 10; i++ {
    if (i % 2 == 0) {
        continue  // skip even numbers
    }
    process(i)
}
```

### 9.7 Error Handling Statements

Fly uses a unique error handling mechanism based on `fail` and `handle` keywords, which differs from traditional try-catch exception handling found in other languages.

**Key Differences from Try-Catch:**
- **`fail`** throws an exception (similar to `throw`)
- **`handle`** catches the exception (similar to `try-catch`)
- The `error` type is used to capture exception information
- More concise syntax with implicit error propagation

#### 9.7.1 The Error Type

The `error` type is a built-in type used to represent exceptions and error states.

**Declaration:**
```fly
error err           // Declares an error variable
error myError       // Error variable to capture exceptions
```

#### 9.7.2 Fail Statement

The `fail` keyword throws an exception. You can fail with nothing, an integer, a string, or an object.

**Syntax:**
```
FailStmt ::= 'fail' [ Expression ]
```

**Examples:**

```fly
// 1. Fail without a value (void failure)
void err0() {
    fail                    // Simple failure
    return false            // Never reached
}

// 2. Fail with an integer error code
int err1() {
    fail 404                // Fail with error code
    return 0                // Never reached
}

// 3. Fail with a string error message
string err2() {
    fail "Error occurred"   // Fail with message
    return ""               // Never reached
}

// 4. Fail with an object or expression
void validateAge(int age) {
    if (age < 0) {
        fail "Age cannot be negative"
    }
    if (age > 150) {
        fail 1001           // Custom error code
    }
}
```

**Fail Statement Behavior:**
- Immediately terminates the current function
- Propagates the exception to the caller
- Can carry data: nothing (void), integers, strings, or objects
- Any code after `fail` is unreachable

#### 9.7.3 Handle Statement

The `handle` keyword catches exceptions thrown by `fail`. It executes a block of code and captures any failures.

**Syntax:**
```
HandleStmt ::= [ 'error' Identifier '=' ] 'handle' ( Statement | Block )
```

**Forms of Handle:**

**1. Simple Handle (No Error Capture):**
```fly
void main() {
    // Just handle the exception, ignore details
    handle err0()
    
    // Handle a block of code
    handle {
        riskyOperation()
        anotherRiskyCall()
    }
}
```

**2. Handle with Error Capture:**
```fly
void main() {
    // Capture error in variable of type 'error'
    bool b = false
    error err0 = handle { 
        b = err0() 
    }
    
    // The error variable contains exception information
    if (err0) {
        // Handle the error
    }
}
```

**3. Handle with Return Value:**
```fly
int processData() {
    int i = 0
    error err1 = handle { 
        i = err1()      // err1() may fail with integer
    }
    
    if (err1) {
        // Error occurred, err1 contains the error
        return -1
    }
    return i
}
```

**4. Handle String Errors:**
```fly
void processText() {
    string s = ""
    error err2 = handle { 
        s = err2()      // err2() may fail with string
    }
    
    if (err2) {
        // err2 contains the string error message
    }
}
```

#### 9.7.4 Complete Error Handling Examples

**Example 1: Simple Void Error Handling**
```fly
void err0() {
    fail                    // Throw exception
}

void main() {
    handle err0()           // Catch exception
    // Application continues and returns 0 (success)
}
```

**Note:** In `main()`, if you don't handle the error, the application will automatically return exit code 1 (failure). When the error is handled (as shown above), the application returns 0 (success). See [Section 5.5: The Main Function](#55-the-main-function) for details.

**Example 2: Integer Error Codes**
```fly
int divide(int a, int b) {
    if (b == 0) {
        fail 1001           // Error code for division by zero
    }
    return a / b
}

void calculate() {
    int result = 0
    error divErr = handle {
        result = divide(10, 0)
    }
    
    if (divErr) {
        // Handle division error
        // divErr contains error code 1001
    }
}
```

**Example 3: String Error Messages**
```fly
string loadFile(string path) {
    if (path == "") {
        fail "Invalid file path"
    }
    // ... load file logic
    return content
}

void processFile() {
    string content = ""
    error fileErr = handle {
        content = loadFile("")
    }
    
    if (fileErr) {
        // fileErr contains "Invalid file path"
    }
}
```

**Example 4: Multiple Operations in Handle Block**
```fly
void complexOperation() {
    bool success = false
    int value = 0
    string data = ""
    
    error err = handle {
        success = operation1()  // May fail
        value = operation2()    // May fail
        data = operation3()     // May fail
    }
    
    if (err) {
        // Any of the operations failed
        // err contains the error information
    } else {
        // All operations succeeded
    }
}
```

#### 9.7.5 Error Handling Patterns

**Pattern 1: Graceful Degradation**
```fly
int getValue() {
    int result = 0
    error err = handle {
        result = riskyOperation()
    }
    
    if (err) {
        return -1   // Default value on error
    }
    return result
}
```

**Pattern 2: Error Propagation**
```fly
void caller() {
    // If handle captures an error, you can re-fail
    error err = handle {
        mayFail()
    }
    
    if (err) {
        fail        // Propagate error to caller
    }
}
```

**Pattern 3: Logging and Recovery**
```fly
void process() {
    error err = handle {
        criticalOperation()
    }
    
    if (err) {
        // Log the error
        log("Error occurred")
        
        // Attempt recovery
        fallbackOperation()
    }
}
```

#### 9.7.6 Comparison with Try-Catch

| Feature | Fly (fail/handle) | Traditional (try-catch) |
|---------|-------------------|-------------------------|
| Throw exception | `fail` | `throw` |
| Catch exception | `handle` | `try-catch` |
| Error variable | `error err = handle { }` | `catch (Exception e) { }` |
| No error value | `fail` | `throw` |
| Error with value | `fail 404` or `fail "msg"` | `throw new Exception(msg)` |
| Syntax | Concise, inline | Verbose, block-based |
| Error type | Built-in `error` type | Exception classes |

**Summary:**
- **fail** = throw an exception (void, int, string, or object)
- **handle** = catch exceptions in a block
- **error** = type to store exception information
- More concise than traditional try-catch
- Supports multiple error payload types

---

## 10. Namespaces and Imports

### 10.1 Namespace Declaration

Namespaces organize code and prevent name conflicts.

**Syntax:**
```
Namespace ::= 'namespace' Identifier ( '.' Identifier )*
```

**Examples:**
```fly
// Single namespace
namespace mylib

// Nested namespace (dotted notation)
namespace my.library

namespace company.project.module
```

**Rules:**
- A namespace declaration must appear before any imports or top-level declarations
- Only one namespace declaration per file
- If no namespace is declared, a default namespace based on the filename is used

### 10.2 Import Declaration

Imports make symbols from other namespaces available.

**Syntax:**
```
Import ::= 'import' Identifier ( '.' Identifier )* [ 'as' Identifier ( '.' Identifier )* ]
```

**Examples:**
```fly
// Simple import
import utils

// Nested namespace import
import my.library

// Import with alias
import standard as std
import external.package as pkg

// Multiple imports
import utils
import helpers
import data.models
```

### 10.3 Using Imported Symbols

**Examples:**
```fly
// File: utils.fly
namespace utils

public int getB() {
    return 10
}
```

```fly
// File: main.fly
import utils

void main() {
    int value = utils.getB()
}
```

```fly
// With alias
import standard as std

void process() {
    std.doSomething()
}
```

---

## 11. Modifiers

### 11.1 Visibility Modifiers

Control the accessibility of declarations.

| Modifier    | Scope                                          | Applies To                    |
|-------------|------------------------------------------------|-------------------------------|
| `private`   | Only within the same file/class                | Functions, classes, members   |
| `protected` | Within the class and derived classes           | Class members                 |
| `public`    | Accessible from anywhere                       | Functions, classes, members   |
| (default)   | Package-private (same namespace)               | Functions, classes            |

**Examples:**
```fly
// Private function
private void internalHelper() {}

// Protected member
class Base {
    protected int value
}

// Public class
public class PublicAPI {
    public void exportedMethod() {}
}

// Default visibility
void packageFunction() {}
class DefaultClass {}
```

### 11.2 Constant Modifier

The `const` modifier marks values as immutable.

**Examples:**
```fly

// Constant function parameter
void process(const int size) {
    // size cannot be modified
}

// Constant local variable
void func() {
    const int limit = 50
    // limit = 100  // Error: cannot modify const
}
```

### 11.3 Static Modifier

The `static` modifier creates class-level members.

**Examples:**
```fly
class Counter {
    static int totalCount = 0
    
    public static int getTotal() {
        return totalCount
    }
    
    public void increment() {
        totalCount++
    }
}

// Usage
int total = Counter.getTotal()
```

### 11.4 Combining Modifiers

Multiple modifiers can be combined.

**Examples:**
```fly
// In a class context
class Configuration {
    // Public constant (class-level)
    public const int BUFFER_SIZE = 1024
    
    // Private static field
    private static int instanceCounter = 0
    
    // Public static constant
    public static const string APP_NAME = "FlyApp"
}

// In a function
void process() {
    // Constant local variable
    const int maxRetries = 3
}
```

---

## 12. Comments

### 12.1 Line Comments

Line comments start with `//` and continue to the end of the line.

**Examples:**
```fly
// This is a line comment
int value = 42  // End-of-line comment

// Multiple line comments
// can be used for
// multi-line documentation
```

### 12.2 Block Comments

Block comments are enclosed between `/*` and `*/`.

**Examples:**
```fly
/* This is a block comment */

/*
 * Multi-line block comment
 * for detailed documentation
 */

int calculate() {
    /* inline comment */ return 0
}
```

**Note:** Block comments can span multiple lines and are preserved by the parser for documentation purposes.

---

## 13. Grammar Summary

### 13.1 Program Structure

```
Program         ::= [ Namespace ] Import* TopDecl*

Namespace       ::= 'namespace' Name ( '.' Name )*

Import          ::= 'import' Name ( '.' Name )* [ 'as' Name ( '.' Name )* ]

TopDecl         ::= Comment 
                  | ClassDecl 
                  | EnumDecl 
                  | FunctionDecl

Modifiers       ::= ( 'public' | 'private' | 'protected' | 'const' | 'static' )*
```

### 13.2 Type System

```
Type            ::= BuiltinType 
                  | NamedType 
                  | ArrayType

BuiltinType     ::= 'void' | 'bool' | 'byte' | 'char' 
                  | 'short' | 'ushort' | 'int' | 'uint' 
                  | 'long' | 'ulong' | 'float' | 'double' 
                  | 'string' | 'error'

NamedType       ::= Name ( '.' Name )*

ArrayType       ::= Type '[' [ Expression ] ']'
```

### 13.3 Declarations

```
ClassDecl       ::= Modifiers ( 'class' | 'struct' | 'interface' ) 
                    Identifier [ ':' BaseList ] '{' ClassMember* '}'

EnumDecl        ::= Modifiers 'enum' Identifier [ ':' BaseType ] '{' EnumEntryList '}'

EnumEntryList   ::= EnumEntry ( ',' EnumEntry )*

EnumEntry       ::= Identifier

FunctionDecl    ::= Modifiers Type Identifier '(' [ ParamList ] ')' ( Block | ';' )

ParamList       ::= Param ( ',' Param )*

Param           ::= [ 'const' ] Type Identifier
```

### 13.4 Statements

```
Statement       ::= Block 
                  | IfStmt 
                  | SwitchStmt 
                  | WhileStmt 
                  | ForStmt
                  | ReturnStmt 
                  | BreakStmt 
                  | ContinueStmt 
                  | FailStmt 
                  | HandleStmt
                  | ExprStmt 
                  | VarDeclStmt 
                  | AssignStmt

Block           ::= '{' Statement* '}'

IfStmt          ::= 'if' [ '(' ] Expr [ ')' ] Statement 
                    ( 'elsif' [ '(' ] Expr [ ')' ] Statement )* 
                    [ 'else' Statement ]

SwitchStmt      ::= 'switch' [ '(' ] Expr [ ')' ] '{' 
                    CaseClause* [ DefaultClause ] '}'

WhileStmt       ::= 'while' [ '(' ] Expr [ ')' ] Statement

ForStmt         ::= 'for' VarDecl ( ',' VarDecl )* ';' Expr ';' 
                    Expr ( ',' Expr )* Statement

ReturnStmt      ::= 'return' [ Expr ]

BreakStmt       ::= 'break'

ContinueStmt    ::= 'continue'

FailStmt        ::= 'fail' [ Expr ]

HandleStmt      ::= [ 'error' Identifier '=' ] 'handle' ( Statement | Block )

VarDeclStmt     ::= Modifiers Type Identifier [ '=' Expr ]

AssignStmt      ::= Identifier AssignOp Expr
```

### 13.5 Expressions

```
Expression      ::= TernaryExpr

TernaryExpr     ::= LogicalOrExpr [ '?' Expr ':' Expr ]

LogicalOrExpr   ::= LogicalAndExpr ( '||' LogicalAndExpr )*

LogicalAndExpr  ::= BitwiseOrExpr ( '&&' BitwiseOrExpr )*

BitwiseOrExpr   ::= BitwiseXorExpr ( '|' BitwiseXorExpr )*

BitwiseXorExpr  ::= BitwiseAndExpr ( '^' BitwiseAndExpr )*

BitwiseAndExpr  ::= EqualityExpr ( '&' EqualityExpr )*

EqualityExpr    ::= RelationalExpr ( ( '==' | '!=' ) RelationalExpr )*

RelationalExpr  ::= ShiftExpr ( ( '<' | '>' | '<=' | '>=' ) ShiftExpr )*

ShiftExpr       ::= AddExpr ( ( '<<' | '>>' ) AddExpr )*

AddExpr         ::= MultExpr ( ( '+' | '-' ) MultExpr )*

MultExpr        ::= UnaryExpr ( ( '*' | '/' | '%' ) UnaryExpr )*

UnaryExpr       ::= ( '++' | '--' | '!' | '-' | '+' ) UnaryExpr 
                  | PostfixExpr

PostfixExpr     ::= PrimaryExpr ( '++' | '--' | '(' ArgList ')' 
                  | '[' Expr ']' | '.' Identifier )*

PrimaryExpr     ::= Literal 
                  | Identifier 
                  | '(' Expr ')' 
                  | 'new' Identifier '(' ArgList ')'
                  | ArrayValue

ArrayValue      ::= '{' [ Expr ( ',' Expr )* ] '}'

AssignOp        ::= '=' | '+=' | '-=' | '*=' | '/=' | '%=' 
                  | '&=' | '|=' | '^=' | '<<=' | '>>='
```

---

## 14. Complete Example

Here's a comprehensive example demonstrating various Fly language features:

```fly
namespace myapp

import utils
import data.models as models


// Enum declaration
public enum Status {
    IDLE, RUNNING, PAUSED, STOPPED
}

// Class declaration
public class Application {
    // Private fields
    private string name
    private int value
    private Status currentStatus
    
    // Static field
    static int instanceCount = 0
    
    // Public method
    public void initialize(string appName) {
        name = appName
        value = 0
        currentStatus = Status.IDLE
        instanceCount++
    }
    
    // Public method with error handling
    public int process() {
        int result = 0
        error err = handle {
            result = calculateResult()
        }
        
        if (err) {
            // Error occurred, return default value
            return -1
        }
        return result
    }
    
    // Private helper method that may fail
    private int calculateResult() {
        if (value < 0) {
            fail "Invalid value"     // Fail with string message
        }
        if (value > 1000) {
            fail 999                 // Fail with error code
        }
        return value + utils.getB()
    }
    
    // Method demonstrating void error handling
    public void validate() {
        error validationErr = handle {
            if (name == "") {
                fail "Name cannot be empty"
            }
        }
        
        if (validationErr) {
            currentStatus = Status.STOPPED
        }
    }
    
    // Getter method
    public string getName() {
        return name
    }
    
    // Setter method
    public void setValue(int newValue) {
        value = newValue
    }
    
    // Static method
    public static int getInstanceCount() {
        return instanceCount
    }
}

// Structure declaration
public struct Point {
    int x
    int y
    
    public int distanceSquared() {
        return x * x + y * y
    }
}

// Main entry point
// Note: main() automatically returns 0 if all errors are handled,
// or returns 1 if an unhandled error occurs
void main() {
    // Create application instance
    Application app = new Application()
    app.initialize("MyApp")
    
    // Error handling example: validate the application
    handle app.validate()
    
    // Set status
    Status status = Status.RUNNING
    
    // Control flow with error handling
    if (status == Status.RUNNING) {
        int result = 0
        error processErr = handle {
            result = app.process()
        }
        
        if (processErr) {
            // Handle error gracefully
            status = Status.STOPPED
        } else {
            handleResult(result)
        }
    } elsif (status == Status.PAUSED) {
        // Handle paused state
    } else {
        // Handle other states
    }
    
    // Loop through array
    int[] numbers = {1, 2, 3, 4, 5}
    for int i = 0; i < 5; i++ {
        processNumber(numbers[i])
    }
    
    // While loop
    int count = 0
    while (count < 10) {
        count++
    }
    
    // Switch statement
    switch (count) {
        case 10:
            // count is 10
            break
        default:
            // other value
    }
    
    // Create structure
    Point p = new Point()
    p.x = 10
    p.y = 20
    
    // Error handling with structure
    error distErr = handle {
        int dist = p.distanceSquared()
        if (dist > 1000) {
            fail "Distance too large"
        }
    }
}

// Private helper function
private void handleResult(int result) {
    while (result > 0) {
        result--
    }
}

// Function with multiple parameters
private void processNumber(int num) {
    if (num % 2 == 0) {
        // even number
    } else {
        // odd number
    }
}
```

---

## 15. Best Practices

### 15.1 Naming Conventions

- **Classes, Structs, Enums**: Use PascalCase (e.g., `MyClass`, `StatusType`)
- **Functions, Variables**: Use camelCase (e.g., `calculateTotal`, `userName`)
- **Constants**: Use UPPER_SNAKE_CASE (e.g., `MAX_SIZE`, `DEFAULT_VALUE`)
- **Private members**: Prefix with underscore or use clear naming (e.g., `_internal`, `privateHelper`)

### 15.2 Code Organization

- One namespace per file
- Group related functionality in the same namespace
- Use imports to reference external code
- Keep functions focused and small

### 15.3 Error Handling

- Use `fail` for unrecoverable errors
- Use `handle` blocks to catch and recover from errors
- Validate inputs at function boundaries

### 15.4 Comments

- Use line comments for brief explanations
- Use block comments for detailed documentation
- Document public APIs thoroughly
- Explain complex algorithms and business logic

---

## Appendix A: Reserved Keywords

All keywords are reserved and cannot be used as identifiers:

```
as          bool        break       byte        case
char        class       const       continue    default
double      elsif       else        enum        error
fail        false       float       for         handle
if          import      interface   int         long
namespace   new         null        private     protected
public      return      short       static      string
struct      switch      true        uint        ulong
ushort      void        while
```

---

## Appendix B: Operator Precedence

From highest to lowest precedence:

1. Postfix: `++`, `--`, `()`, `[]`, `.`
2. Unary: `++`, `--`, `!`, `-`, `+` (prefix)
3. Multiplicative: `*`, `/`, `%`
4. Additive: `+`, `-`
5. Shift: `<<`, `>>`
6. Relational: `<`, `>`, `<=`, `>=`
7. Equality: `==`, `!=`
8. Bitwise AND: `&`
9. Bitwise XOR: `^`
10. Bitwise OR: `|`
11. Logical AND: `&&`
12. Logical OR: `||`
13. Ternary: `?:`
14. Assignment: `=`, `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`, `>>=`

---

## Appendix C: Error Handling Quick Reference

Fly uses `fail` and `handle` keywords for error handling, which differs from traditional try-catch mechanisms.

### Quick Comparison

| Concept | Fly Syntax | Traditional (Java/C++) |
|---------|------------|------------------------|
| Throw exception | `fail` | `throw` |
| Throw with message | `fail "Error message"` | `throw new Exception("Error message")` |
| Throw with code | `fail 404` | `throw 404` or custom exception |
| Catch exception | `handle { ... }` | `try { ... } catch { ... }` |
| Catch with variable | `error err = handle { ... }` | `catch (Exception err) { ... }` |
| Error type | `error` | `Exception` or custom class |

### Common Patterns

```fly
// Pattern 1: Simple fail
void operation() {
    fail                        // Throw exception
}

// Pattern 2: Fail with integer code
int check() {
    fail 404                    // Error code
}

// Pattern 3: Fail with string message
string load() {
    fail "File not found"       // Error message
}

// Pattern 4: Simple handle
handle operation()              // Catch and ignore

// Pattern 5: Handle with error capture
error err = handle {
    riskyOperation()
}
if (err) {
    // Handle error
}

// Pattern 6: Handle with return value
int result = 0
error err = handle {
    result = computation()
}
```

### Error Types

- **void**: `fail` (no value)
- **integer**: `fail 404`, `fail 500`, `fail -1`
- **string**: `fail "Error message"`, `fail "Not found"`
- **object**: `fail errorObject`

### Key Points

1. **`fail`** immediately terminates function execution
2. **`handle`** catches exceptions in the enclosed block
3. **`error`** type stores exception information
4. Multiple operations can be wrapped in a single `handle` block
5. Error handling is more concise than traditional try-catch
6. No exception type hierarchy needed—use simple values
7. **`main()` function:** Unhandled errors cause the application to return exit code 1; handled errors allow return code 0

### Main Function and Exit Codes

The `main()` function has special error handling behavior:

```fly
void main() {
    // If no error occurs or all errors are handled: returns 0
    // If an unhandled error occurs: returns 1
}
```

**Examples:**

```fly
// Returns 0 (success)
void main() {
    handle mayFail()
}

// Returns 1 (failure)
void main() {
    mayFail()  // Error not handled
}

// Returns 0 (success) - error is caught and handled
void main() {
    error err = handle {
        riskyOperation()
    }
    if (err) {
        // Handle gracefully
    }
}
```

---

**© Fly Project - https://flylang.org**  
**Licensed under Apache License v2.0**  
**Documentation Version 1.0 - December 2025**

