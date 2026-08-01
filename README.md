# Anvil

> A robust, low-overhead library that brings Object-Oriented Programming (OOP), RAII, generic programming, file streams, and C++-style exception handling to C — built on Clang's Blocks extension.

---

## Table of Contents

- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Features](#features)
  - [1. Object-Oriented Programming](#1-object-oriented-programming)
  - [2. Interfaces & Inheritance](#2-interfaces--inheritance)
  - [3. RAII Smart Pointers](#3-raii-smart-pointers)
  - [4. Defer](#4-defer)
  - [5. Exception Handling](#5-exception-handling)
  - [6. File Streams](#6-file-streams)
  - [7. Reflection](#7-reflection)
  - [8. Generic Programming](#8-generic-programming)
  - [9. Private Members](#9-private-members)
- [Experimental Flags](#experimental-flags)
- [Build Flags Reference](#build-flags-reference)
- [API Reference](#api-reference)
- [License](#license)

---

## Prerequisites

- **Clang** (GCC is not supported — see `oopgcc.h`)
- **Blocks runtime**: compile with `-fblocks` and link `-lBlocksRuntime`

```sh
clang -fblocks -lBlocksRuntime your_program.c
```

---

## Quick Start

```c
#include "oop.h"           // OOP macros (auto-selects clang backend)
#include "raii.h"          // sptr (smart pointer / RAII)
#include "defer.h"         // defer (C2Y-compatible)
#include "exception.h"     // try / catch / throw
#include "fstream.h"       // file stream with method chaining
#include "reflect.h"       // runtime reflection
#include "generic/pair.h"  // generic pair<T, U>
```

---

## Features

### 1. Object-Oriented Programming

Define classes with fields and methods, constructors and destructors. Each instance carries an `Object` header at a negative offset, enabling runtime type checks and destructor dispatch.

**Macros used:** `class()`, `method()`, `ctor()`, `dtor()`, `bind()`, `unbind()`, `new()`, `delete()`, `instanceof()`

```c
#include <stdio.h>
#include "oop.h"

class (A)
{
    int a;
    float b;
    method(int, get_a, void);
    method(float, get_b, void);
};

ctor(A, int a, float b)
{
    this->a = a;
    this->b = b;
    bind(this, get_a)
    {
        return this->a;
    };
    bind(this, get_b)
    {
        return this->b;
    };
    return 0;
};

dtor(A)
{
    unbind(this, get_a);
    unbind(this, get_b);
    printf("~A\n");
};

void test(void)
{
    A *a = new(A, 1, 2.0f);
    printf("a->a = %d, a->b = %f\n", a->get_a(), a->get_b());
    printf("Is a instance of A? %d\n", instanceof(A, a));
    delete(A, a);
}
```

#### With RAII Smart Pointers

```c
void test_raii(void)
{
    sptr a = new(sptr_t, new(A, 1, 2.0f));
    A *obj = a->get_ptr();
    printf("a->a = %d, a->b = %f\n", obj->get_a(), obj->get_b());
    // a automatically released when it goes out of scope
}
```

---

### 2. Interfaces & Inheritance

Define abstract contracts with `interface()` and implement them via `extends()`. Dispatch is manual (operation tables), giving you full control over vtable layout.

**Macros used:** `class()`, `interface()`, `extends()`, `bind()`, `divfn_cast()`

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
    extends(Numeric);       // Integer inherits Numeric
    int v;
    method(Integer *, add, Integer *);
};

ctor(Integer, int v)
{
    this->v = v;
    this->base.ops.add = Block_copy(^(Numeric *a) {
        auto new_int = new(Integer, this->v + ((Integer *)a)->v);
        return (Numeric *)new_int;
    });
    bind(this, add, ^(Integer *a) {
        return divfn_cast(Numeric, Integer, this->base.ops.add, a);
    });
    return 0;
};
```

| Macro | Purpose |
|---|---|
| `interface(name)` | Declare an abstract type with an operation table |
| `extends(type)` | Inherit all fields from a base struct |
| `divfn_cast(base_t, div_t, fn, ...)` | Cast a base-type function pointer to a derived-type one |

---

### 3. RAII Smart Pointers

`sptr` provides automatic memory management via reference counting and `[[gnu::cleanup]]`. When a `sptr` variable goes out of scope, its reference count is decremented — the managed object is freed when the count reaches zero.

```c
#include "raii.h"

void example(void)
{
    sptr a = new(sptr_t, new(A, 10, 20.0f));
    sptr b = a->borrow();    // increment refcount
    // ... use a->get_ptr() or b->get_ptr()
    // b goes out of scope → refcount decremented
    // a goes out of scope → refcount reaches 0 → object freed
}
```

| Type / Macro | Purpose |
|---|---|
| `sptr_t` | Smart pointer class (holds `sptr_priv` with refcount + raw pointer) |
| `sptr` | Variable declaration with `[[gnu::cleanup]]` auto-release |
| `new(sptr_t, ptr)` | Wrap a raw `new(...)` pointer in a smart pointer |
| `a->borrow()` | Borrow a reference (increment refcount) |
| `a->get_ptr()` | Unwrap the raw pointer |

---

### 4. Defer

`defer` is a **C2Y** feature ([ISO/IEC TS 25755](https://www.open-std.org/jtc1/sc22/wg14/)), scheduled to be standardized in the next C revision. Anvil provides a **lightweight macro** that is source-compatible with C2Y's `defer` syntax. When the compiler already supports C2Y (`__STDC_DEFER_TS25755__` is defined), the macro transparently delegates to `<stddefer.h>`'s native `_Defer` implementation. Otherwise, it falls back to a Clang Blocks-based implementation using `[[gnu::cleanup]]`.

```c
#include "defer.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    int *arr = malloc(sizeof(int) * 10);
    defer
    {
        printf("free\n");
        free(arr);
    };

    // use arr ...
    for (int i = 0; i < 10; i++)
        arr[i] = i;
    printf("arr[5] = %d\n", arr[5]);
    // arr is freed automatically when main() returns
}
```

Combine `defer` with `fstream`:

```c
#include "fstream.h"

int main()
{
    fstream *f = new(fstream, "log.txt", "w");
    defer
    {
        printf("fclose\n");
        f->close();
    };

    f->write("hello")->write(" world")->flush();
    // file closed automatically on scope exit
}
```

| Implementation path | Conditions |
|---|---|
| Native C2Y `_Defer` | `__STDC_DEFER_TS25755__` is defined |
| Anvil fallback | Uses `[[gnu::cleanup]]` + Clang Blocks |

---

### 5. Exception Handling

C++-style `try`/`catch`/`throw` built on `setjmp`/`longjmp` with a thread-local exception handler stack. Use the annotation macros `throws` and `noexcept` for documentation intent (they expand to nothing).

**Macros used:** `try`, `catch`, `throw()`, `throws`, `noexcept`, `exception_as()`, `Exception()`

```c
#include "exception.h"
#include <stdio.h>

int main() throws
{
    try
    {
        throw(Exception("Something went wrong.\n"));
    }
    catch
    {
        exception_as(e);
        printf("%s", e->msg);
    }
    // uncaught exceptions will abort
}
```

| Macro | Purpose |
|---|---|
| `throws` | Annotation hint — marks a function that may throw (expands to nothing) |
| `noexcept` | Annotation hint — marks a function that does not throw (expands to nothing) |
| `try` | Begin a guarded block |
| `catch` | Catch any exception thrown in the `try` block |
| `throw(e)` | Throw an exception (noreturn) |
| `Exception(msg)` | Allocate an `Exception` with a message string |
| `exception_as(name)` | Bind the caught exception to a local variable |

---

### 6. File Streams

A fluent file I/O wrapper with method chaining. Supports `write`, `flush`, `close`, `rewind`, `seek`, `tell`, and direct `raw_file` access.

**Macros used:** `class()`, `method()`, `ctor()`, `dtor()`, `bind()`, `new()`, `delete()`

```c
#include "fstream.h"

int main()
{
    fstream *f = new(fstream, "a.txt", "w");
    f
        ->write("hello")
        ->write(" world")
        ->flush();
    f->close();
    delete(fstream, f);
}
```

| Member | Type | Description |
|---|---|---|
| `raw_file` | `FILE *` | Direct access to the underlying `FILE *` |
| `write(char *s)` | `fstream *` | Write a C string; returns `this` for chaining |
| `flush(void)` | `fstream *` | Flush the buffer; returns `this` for chaining |
| `close(void)` | `void` | Close the underlying `FILE *` |
| `rewind(void)` | `fstream *` | Rewind to the beginning |
| `seek(long, int)` | `fstream *` | Seek to an offset |
| `tell(void)` | `long` | Get current position |

---

### 7. Reflection

Runtime field access and method invocation by name, using compile-time `offsetof` and a static metadata registry (auto-registered via `__attribute__((constructor))`).

**Macros used:** `reflect()`, `rfl_member()`, `getfield()`, `setfield()`, `callmeth()`

```c
#include "reflect.h"
#include <stdio.h>

class (A)
{
    int a;
    method(int, get_a, void);
};

ctor(A, int a)
{
    this->a = a;
    bind(this, get_a)
    {
        return this->a;
    };
    return 0;
};

dtor(A) {};

reflect(
    A,
    rfl_member(A, a),
    rfl_member(A, get_a)
);

int main()
{
    void *a = new(A, 5);
    printf("%d\n", getfield(A, a, a));      // read field "a" by name   → 5
    printf("%d\n", callmeth(A, a, get_a));   // call method "get_a" by name → 5
}
```

| Macro | Purpose |
|---|---|
| `reflect(cls, ...)` | Register reflection metadata for a class |
| `rfl_member(cls, member)` | Describe one member (name + offset) |
| `getfield(cls, obj, mem)` | Read a field by name |
| `setfield(cls, obj, mem, val)` | Write a field by name |
| `callmeth(cls, obj, mem, ...)` | Invoke a method by name |

---

### 8. Generic Programming

Type-safe generic data structures via C macros. Includes a generic `pair<T, U>`.

```c
#include "generic/pair.h"

int main()
{
    pair(int, float) *p = new_pair(int, float, 42, 3.14f);
    printf("first = %d, second = %f\n", p->first, p->second);
    pair_destroy(p);
}
```

| Macro | Purpose |
|---|---|
| `pair(T, U)` | Declare a pair struct type with fields `first: T` and `second: U` |
| `new_pair(T, U, first, second)` | Allocate and initialize a pair |
| `pair_init(this, first_, second_)` | Initialize an already-allocated pair |
| `pair_destroy(this)` | Free a heap-allocated pair |

---

### 9. Private Members

Use `private()` to hide internal fields behind an opaque pointer. Access private data through `this->priv`.

```c
#include "oop.h"

class (A)
{
    private(A)
    {
        int a;
        float b;
    };
    method(int, get_a, void);
    method(float, get_b, void);
};

ctor(A, int a, float b)
{
    this->priv->a = a;
    this->priv->b = b;
    bind(this, get_a)
    {
        return this->priv->a;
    };
    bind(this, get_b)
    {
        return this->priv->b;
    };
    return 0;
};
```

---

## Experimental Flags

Defined in `flags.h`. Override with `-D` at compile time.

| Flag | Default | Effect |
|---|---|---|
| `EXPER_DELETE` | `1` | `delete(obj)` without specifying the type — infers type from the `Object` header |
| `EXPER_BIND` | `1` | Out-of-block syntax for `bind`: `bind(self, meth, int x) { ... }` (explicit parameter list) |

---

## Build Flags Reference

| Flag | Required? | Purpose |
|---|---|---|
| `-fblocks` | **Required** | Enable Clang Blocks extension |
| `-lBlocksRuntime` | **Required** | Link the Blocks runtime library on Linux and Windows|
| `-DEXPER_DELETE=0` | Optional | Disable type-less `delete(obj)` |
| `-DEXPER_BIND=0` | Optional | Disable extended `bind` syntax |

Minimal build command:

```sh
clang -fblocks -lBlocksRuntime your_program.c
```

---

---

## About Windows

**Anvil** is compatible with **LLVM-MINGW Clang** and has conflicts with Clang **which provided by Visual Studio**. **libBlocksRuntime** for Windows is provided in Anvil.

---

## API Reference

### Class Definition

| Macro | Purpose |
|---|---|
| `class(name)` | Declare a class struct with automatic `dtor_decl` and typedef |
| `interface(name)` | Declare an interface (abstract type with operation table) |
| `extends(type)` | Inherit from a base class (single inheritance) |
| `method(ret, name, ...)` | Declare a method (Block function pointer) |
| `private(cls) { ... }` | Opaque private data section |

### Lifecycle

| Macro | Purpose |
|---|---|
| `ctor(name, ...)` | Define constructor; `return 0` on success |
| `dtor(name)` | Define destructor (releases bound methods + cleanup) |
| `new(name, ...)` | Allocate + construct an object on the heap |
| `delete(name, obj)` | Call destructor + free (type-safe, or type-less with `EXPER_DELETE=1`) |
| `new(sptr_t, ptr)` | Wrap a heap object in a smart pointer |

### Method Binding

| Macro | Purpose |
|---|---|
| `bind(self, method) { ... }` | Bind a method implementation (Block); auto-releases previous binding |
| `bind(self, method, ^(...) { ... })` | Bind with an explicit Block expression |
| `unbind(self, method)` | Release a bound method |

### Introspection

| Macro | Purpose |
|---|---|
| `instanceof(type, obj)` | Runtime type check (1 if `cls_name` matches) |
| `to_object(obj)` | Get the internal `Object` header from an instance pointer |

### Blocks Helpers

| Macro | Purpose |
|---|---|
| `lambda(ret, args, body)` | Inline lambda expression returning a Block |
| `lbd_t(ret, args)` | Type alias for a Block prototype |

---

## License

MIT — see [LICENSE](./LICENSE).
