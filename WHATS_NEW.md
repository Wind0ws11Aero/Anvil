# What's New in Anvil 2.0

Anvil 2.0 is a major release that introduces new language-level constructs, refines the OOP system, and improves ergonomics across the board. This document covers the key changes from 1.x to 2.0.

---

## Breaking Changes

### `class()` vs `interface()` — The OOP Split

In 1.x, `class()` behaved like an interface — it declared a struct with typedef but did not attach any constructor/destructor glue. In 2.0, `class()` now automatically generates destructor declarations (`dtor_decl`) and the full OOP lifecycle is expected. For the old bare-struct-with-typedef behavior, use the new `interface()` macro.

**1.x:**
```c
class (A)
{
    int a;
    method(int, get_a, void);
};
```

**2.0:**
```c
// Full OOP class with ctor/dtor lifecycle
class (A)
{
    int a;
    method(int, get_a, void);
};

// Bare abstract type (was the old "class")
interface (Numeric)
{
    struct Numeric_ops
    {
        method(Numeric *, add, Numeric *);
    } ops;
};
```

### `private()` Syntax Change

`private()` now uses a braced block instead of variadic arguments. Access remains through `this->priv`.

**1.x:**
```c
private(A, int a; float b;);
```

**2.0:**
```c
private(A)
{
    int a;
    float b;
};
```

### `Object` Header Redesign

The internal metadata header has been upgraded from `object_t` to `Object`. Every object created via `new()` now carries a proper `Object` header containing `cls_name` and `dtor_fn` for runtime type introspection and automatic destructor dispatch. The `struct object_t` typedef is removed — use `Object` instead.

### GCC Support Removed

Anvil now targets **Clang only**. The `oopgcc.h` backend has been removed. The Blocks extension are fundamental to Anvil's design and are Clang-native features.

### Exception System Refactored

`Exception` and `ExceptionHandler` have been rewritten as proper OOP classes (using `class()`, `ctor()`, `bind()`). The old flat-struct approach is gone.

**1.x:**
```c
typedef struct Exception { /* raw struct */ } Exception;
thread_local ExceptionHandler _handler;
```

**2.0:**
```c
class (Exception) { /* full OOP class */ };
class (ExceptionHandler) { /* proper class with push/pop/peek methods */ };
```

---

## New Features

### Defer — C2Y Compatible

`defer` brings Go-style deferred execution to C, source-compatible with the upcoming C2Y standard (ISO/IEC TS 25755). When the compiler defines `__STDC_DEFER_TS25755__`, the macro transparently delegates to `<stddefer.h>`'s native `_Defer`. Otherwise it falls back to a Clang Blocks-based implementation.

```c
#include "defer.h"

int main()
{
    int *arr = malloc(sizeof(int) * 10);
    defer
    {
        free(arr);
    };
    // arr is freed automatically when main() returns
}
```

### Interfaces & Inheritance

New `interface()` macro for declaring abstract types with operation tables, plus `extends()` for single inheritance. Method dispatch is manual, giving you full control over vtable layout. The new `divfn_cast()` safely casts base-type function pointers to derived types.

```c
interface (Numeric)
{
    struct Numeric_ops
    {
        method(Numeric *, add, Numeric *);
    } ops;
};

class (Integer)
{
    extends(Numeric);        // Integer inherits Numeric's interface
    int v;
    method(Integer *, add, Integer *);
};
```

### Type-less `delete()` — EXPERIMENTAL

When `EXPER_DELETE=1` (default in 2.0), you can call `delete(obj)` without specifying the type. The type is inferred from the `Object` header attached to each instance.

```c
A *a = new(A, 1, 2.0f);
delete(a);          // 2.0: no type argument needed
// delete(A, a);    // 1.x: type was required
```

### Extended `bind()` Syntax — EXPERIMENTAL

When `EXPER_BIND=1` (default in 2.0), `bind()` supports an out-of-block parameter list syntax, keeping method signatures directly visible in the binding:

```c
bind(this, write, char *cont)
{
    if (fputs(cont, this->raw_file) < 0) throw(Exception("Error"));
    return this;
};
```

### Feature Flags System

A new `flags.h` header provides compile-time toggles for experimental features:

| Flag | Default | Purpose |
|---|---|---|
| `EXPER_DELETE` | `1` | Type-less `delete(obj)` |
| `EXPER_BIND` | `1` | Extended `bind` syntax with explicit parameters |

Override with `-D`:
```sh
clang -fblocks -lBlocksRuntime -DEXPER_BIND=0 program.c
```

### `throws` / `noexcept` Annotations

New annotation macros for documenting exception guarantees. Both expand to nothing — they serve as human-readable hints and future-proofing for static analysis.

```c
int main() throws
{
    // may throw
}

int safe_fn(void) noexcept
{
    // never throws
}
```

### Blocks Helper Macros

New utility macros for working with Clang Blocks:

| Macro | Purpose |
|---|---|
| `lambda(ret, args, body)` | Inline lambda expression returning a Block |
| `lbd_t(ret, args)` | Type alias for a Block prototype |

```c
lbd_t(int, (int, int)) add = ^int(int a, int b) { return a + b; };
auto predicate = lambda(bool, (int x), { return x > 0; });
```

---

## Migration Guide

### From 1.x to 2.0

1. **Rename `object_t` → `Object`** if you were using it directly.

2. **Replace variadic `private()` with braced `private()`:**
   ```c
   // Before
   private(A, int x; float y;);
   // After
   private(A) { int x; float y; };
   ```

3. **If you used `class()` for bare structs, switch to `interface()`:**
   ```c
   // Before
   class (Shape) { void (*draw)(void); };
   // After
   interface (Shape) { void (*draw)(void); };
   ```

4. **Add `dtor()` to classes that didn't have one.** `class()` now expects a destructor.

5. **Exception code using raw structs must update** to the new OOP-based `Exception` class.

6. **Add `-DEXPER_DELETE=0`** if type-less `delete` breaks your build (unlikely for most codebases).

7. **Add `-DEXPER_BIND=0`** if the new out-of-block bind syntax conflicts with existing macros.

---

## Full Changelog

### Added
- `defer.h` — C2Y-compatible `defer` macro
- `flags.h` — feature flag system (`EXPER_DELETE`, `EXPER_BIND`)
- `interface()` macro — abstract type declarations with operation tables
- `extends()` macro — single inheritance
- `divfn_cast()` — base-to-derived function pointer cast
- `throws` / `noexcept` — exception annotation macros
- `lambda()`, `fptr`, `lbd_t` — Blocks utility macros
- Extended `bind()` syntax with explicit parameter lists
- Type-less `delete()` via runtime type inference
- Object header now stores `dtor_fn` for automatic destructor dispatch
- `delete()` with `EXPER_DELETE=1` infers type from Object header

### Changed
- `class()` now auto-declares destructor (`dtor_decl`) and is distinct from `interface()`
- `private()` changed from variadic args to braced block syntax
- `object_t` → `Object` (header struct rename)
- `Exception` and `ExceptionHandler` refactored as OOP classes
- `new()` attaches full `Object` header with `cls_name` + `dtor_fn`

### Removed
- GCC support (`oopgcc.h` backend)
- `object_t` typedef (replaced by `Object`)

### Fixed
- Destructor declaration issues (`dtor_decl` extern fixes)
- Method binding memory safety (auto-`Block_release` previous binding)
- Reference count race condition in `sptr_t` (switched to `atomic_int`)
