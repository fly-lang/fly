# Language Specification (Design Document)

## 1. General Philosophy

The language is designed with the goal of:

- maximum readability
- explicit control flow
- absence of implicit runtime behaviors
- deterministic error handling
- native and safe multithreading support

There are no runtime exceptions (`throw`, `try-catch`). Errors are not events but **data**.
Functions and methods always and only return an error that is implicitly declared.

---

## 2. Key Concept: Implicit `error`

Every function and method has an **intrinsic and hidden** variable passed by reference:

```
error of type 'error'
```

Characteristics:

- it is always present
- it cannot be declared or shadowed
- it is thread-local
- it represents **the state of the last executed operation (function or method)**
- it is automatically repopulated on each function/method call

The programmer can read it but not declare it.

---

## 3. Structure of `struct error`

```
struct error {
    int     code      // 0 = failure, >0 = valid state
    int8*   msg       // optional pointer to string
    Object* obj       // optional pointer to object
}
```

- `code` is directly usable as boolean validation
- `msg` and `obj` are mutually optional

---

## 4. Exiting a Function

### 4.1
All declared functions are of type `void` and cannot declare a return type.
The only way to exit a function is through `fail` or `return`.

### 4.2 `fail`

```
fail <value>
```

- immediately interrupts the function
- populates `error`

Allowed forms:

- `fail 0`
- `fail 1`
- `fail "message"`
- `fail new Err()`
- `fail error` (reuses the current error)

### 4.3 `return`

```
return
```

- interrupts the function
- **does not modify `error`**
- used for early exit without error

---

## 5. `is_error()`

Builtin function:

```
bool is_error() {
    return error.code > 0
}
```

Usable anywhere to check the state of the last operation.

---

## 6. Function Parameters

- all parameters are **always passed by reference**
- there is no pass by value
- no implicit copying

### 6.1 `const`

```
func(const int a, int b)
```

- `const` prevents modification of the parameter
- const parameters are used as input

### 6.2 Implicit Output

- if the function modifies a parameter, it is valid
- if it does not modify it, no effect
- a modified parameter can be understood as an output parameter

---

## 7. Semantic Rules on Parameters

- double declaration in scope is an error:

```
update(x, x) // semantic error
```

- passing different references to the same object is allowed:

```
update(house, house.bed)
```

- the compiler may emit a warning, but does not forbid the case

---

## 9. Object-Oriented Programming

### 9.1 Methods

- every method has an implicit `error`
- they never declare a return type

### 9.2 Constructors

- they return the instantiated object (only case of return different from error)
- they populate `error`
- the object is passed by reference

Example:

```
House h = new House(120, 2)
if not is_valid():
    print(error.msg)
```

---

## 10. `handle` Block

Construct for handling multiple errors without repeating checks.

```
handle {
    foo()
    bar()
    baz()
} if error {
    fail error
}
```

or adding syntactic sugar:

```
handle {
    foo()
    bar()
    baz()
} or fail
```

### 10.1 Semantics

- monitors `error` within the block
- if a `fail` occurs, the block terminates
- `return` is not intercepted
- no runtime exceptions

---

## 11. Control Flow Rules

1. if you don't check `error`, the flow continues
2. `error` always represents **the last operation**
3. `fail` interrupts
4. `return` interrupts without error
5. `handle` can intercept multiple errors from multiple calls, interrupting the flow at the first error

---

## 12. Conceptual Comparison

| Concept | C | Java | This Language |
|---------|---|------|---------------|
| Errors | errno | exceptions | error + fail |
| Thread | lock | synchronized | sync |
| Runtime crash | yes | yes | no (by design) |
| Flow | implicit | implicit | explicit |

---

## 13. Key Principles

- errors as data
- always readable flow
- no runtime magic
- syntactic simplicity
- local responsibility

---

## 14. Project Status

This document defines the **conceptual version 1.0** of the language.

Possible future extensions:

- thread priority
- override with `sync`
- static analysis on `error`
- async / deferred error

