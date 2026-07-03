# Anvil

> 一个健壮、低开销的 C 语言库，基于 Clang Blocks 扩展，为 C 带来面向对象编程（OOP）、RAII、泛型编程、文件流以及 C++ 风格的异常处理。

---

## 目录

- [前置条件](#前置条件)
- [快速开始](#快速开始)
- [特性](#特性)
  - [1. 面向对象编程](#1-面向对象编程)
  - [2. 接口与继承](#2-接口与继承)
  - [3. RAII 智能指针](#3-raii-智能指针)
  - [4. Defer](#4-defer)
  - [5. 异常处理](#5-异常处理)
  - [6. 文件流](#6-文件流)
  - [7. 反射](#7-反射)
  - [8. 泛型编程](#8-泛型编程)
  - [9. 私有成员](#9-私有成员)
- [实验性标记](#实验性标记)
- [编译标记参考](#编译标记参考)
- [API 参考](#api-参考)
- [许可证](#许可证)

---

## 前置条件

- **Clang**（不支持 GCC — 参见 `oopgcc.h`）
- **Blocks 运行时**：编译时添加 `-fblocks`，链接 `-lBlocksRuntime`

```sh
clang -fblocks -lBlocksRuntime program.c
```

---

## 快速开始

```c
#include "oop.h"           // OOP 宏（自动选择 clang 后端）
#include "raii.h"          // sptr（智能指针 / RAII）
#include "defer.h"         // defer（兼容 C2Y）
#include "exception.h"     // try / catch / throw
#include "fstream.h"       // 文件流，支持链式调用
#include "reflect.h"       // 运行时反射
#include "generic/pair.h"  // 泛型 pair<T, U>
```

---

## 特性

### 1. 面向对象编程

定义带有字段和方法的类，支持构造和析构函数。每个实例在负偏移量处携带 `Object` 头，实现运行时类型检查与析构分发。

**相关宏：** `class()`、`method()`、`ctor()`、`dtor()`、`bind()`、`unbind()`、`new()`、`delete()`、`instanceof()`

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
    printf("a 是 A 的实例吗？%d\n", instanceof(A, a));
    delete(A, a);
}
```

#### 搭配 RAII 智能指针

```c
void test_raii(void)
{
    sptr a = new(sptr_t, new(A, 1, 2.0f));
    A *obj = a->get_ptr();
    printf("a->a = %d, a->b = %f\n", obj->get_a(), obj->get_b());
    // a 离开作用域时自动释放
}
```

---

### 2. 接口与继承

使用 `interface()` 定义抽象契约，通过 `extends()` 实现继承。方法派发为手动操作表，让你完全掌控虚表布局。

**相关宏：** `class()`、`interface()`、`extends()`、`bind()`、`divfn_cast()`

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
    extends(Numeric);       // Integer 继承 Numeric
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

| 宏 | 用途 |
|---|---|
| `interface(name)` | 声明一个带有操作表的抽象类型 |
| `extends(type)` | 继承基类的所有字段 |
| `divfn_cast(base_t, div_t, fn, ...)` | 将基类函数指针转型为派生类函数指针 |

---

### 3. RAII 智能指针

`sptr` 通过引用计数与 `__attribute__((cleanup))` 实现自动内存管理。当 `sptr` 变量离开作用域时，引用计数递减——计数归零时自动释放所管理的对象。

```c
#include "raii.h"

void example(void)
{
    sptr a = new(sptr_t, new(A, 10, 20.0f));
    sptr b = a->borrow();    // 增加引用计数
    // ... 使用 a->get_ptr() 或 b->get_ptr()
    // b 离开作用域 → 引用计数递减
    // a 离开作用域 → 引用计数归零 → 对象释放
}
```

| 类型 / 宏 | 用途 |
|---|---|
| `sptr_t` | 智能指针类（包含 `sptr_priv`，持有引用计数与原始指针） |
| `sptr` | 带 `__attribute__((cleanup))` 自动释放的变量声明 |
| `new(sptr_t, ptr)` | 将原始 `new(...)` 指针包装为智能指针 |
| `a->borrow()` | 借用引用（引用计数 +1） |
| `a->get_ptr()` | 取出原始指针 |

---

### 4. Defer

`defer` 是 **C2Y** 特性（[ISO/IEC TS 25755](https://www.open-std.org/jtc1/sc22/wg14/)），计划在下一次 C 标准修订中正式纳入。Anvil 提供了一个**轻量宏**，其语法与 C2Y 的 `defer` 保持源码兼容。当编译器已支持 C2Y（定义了 `__STDC_DEFER_TS25755__`）时，该宏将直接透传至 `<stddefer.h>` 中的原生 `_Defer` 实现；否则回退到基于 Clang Blocks 与 `__attribute__((cleanup))` 的实现。

```c
#include "defer.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int *arr = malloc(sizeof(int) * 10);
    defer
    {
        printf("free\n");
        free(arr);
    };

    // 使用 arr ...
    for (int i = 0; i < 10; i++)
        arr[i] = i;
    printf("arr[5] = %d\n", arr[5]);
    // main() 返回时 arr 自动释放
}
```

搭配 `fstream` 使用：

```c
#include "fstream.h"

int main(void)
{
    fstream *f = new(fstream, "log.txt", "w");
    defer
    {
        printf("fclose\n");
        f->close();
    };

    f->write("hello")->write(" world")->flush();
    // 离开作用域时文件自动关闭
}
```

| 实现路径 | 条件 |
|---|---|
| 原生 C2Y `_Defer` | 已定义 `__STDC_DEFER_TS25755__` |
| Anvil 回退方案 | 使用 `__attribute__((cleanup))` + Clang Blocks |

---

### 5. 异常处理

基于 `setjmp`/`longjmp` 与线程局部异常处理器栈实现的 C++ 风格 `try`/`catch`/`throw`。可使用 `throws` 和 `noexcept` 标注宏来说明函数意图（它们展开为空）。

**相关宏：** `try`、`catch`、`throw()`、`throws`、`noexcept`、`exception_as()`、`Exception()`

```c
#include "exception.h"
#include <stdio.h>

int main(void) throws
{
    try
    {
        throw(Exception("出错了。\n"));
    }
    catch
    {
        exception_as(e);
        printf("%s", e->msg);
    }
    // 未捕获的异常将触发 abort
}
```

| 宏 | 用途 |
|---|---|
| `throws` | 标注宏——标记可能抛出异常的函数（展开为空） |
| `noexcept` | 标注宏——标记不抛出异常的函数（展开为空） |
| `try` | 开启受保护块 |
| `catch` | 捕获 `try` 块中抛出的任何异常 |
| `throw(e)` | 抛出异常（不返回） |
| `Exception(msg)` | 分配一个包含消息字符串的 `Exception` |
| `exception_as(name)` | 将捕获的异常绑定到局部变量 |

---

### 6. 文件流

支持链式调用的流式文件 I/O 封装。提供 `write`、`flush`、`close`、`rewind`、`seek`、`tell` 以及直接访问底层 `raw_file`。

**相关宏：** `class()`、`method()`、`ctor()`、`dtor()`、`bind()`、`new()`、`delete()`

```c
#include "fstream.h"

int main(void)
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

| 成员 | 类型 | 说明 |
|---|---|---|
| `raw_file` | `FILE *` | 直接访问底层 `FILE *` |
| `write(char *s)` | `fstream *` | 写入 C 字符串；返回 `this` 以支持链式调用 |
| `flush(void)` | `fstream *` | 刷新缓冲区；返回 `this` 以支持链式调用 |
| `close(void)` | `void` | 关闭底层 `FILE *` |
| `rewind(void)` | `fstream *` | 回到文件开头 |
| `seek(long, int)` | `fstream *` | 移动文件指针到指定偏移量 |
| `tell(void)` | `long` | 获取当前文件位置 |

---

### 7. 反射

通过编译期 `offsetof` 与静态元数据注册表（利用 `__attribute__((constructor))` 自动注册），实现按名称访问字段和调用方法。

**相关宏：** `reflect()`、`rfl_member()`、`getfield()`、`setfield()`、`callmeth()`

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

int main(void)
{
    void *a = new(A, 5);
    printf("%d\n", getfield(A, a, a));      // 按名称读取字段 "a"   → 5
    printf("%d\n", callmeth(A, a, get_a));   // 按名称调用方法 "get_a" → 5
}
```

| 宏 | 用途 |
|---|---|
| `reflect(cls, ...)` | 为类注册反射元数据 |
| `rfl_member(cls, member)` | 描述一个成员（名称 + 偏移量） |
| `getfield(cls, obj, mem)` | 按名称读取字段 |
| `setfield(cls, obj, mem, val)` | 按名称写入字段 |
| `callmeth(cls, obj, mem, ...)` | 按名称调用方法 |

---

### 8. 泛型编程

通过 C 宏实现的类型安全泛型数据结构。包含泛型 `pair<T, U>`。

```c
#include "generic/pair.h"

int main(void)
{
    pair(int, float) *p = new_pair(int, float, 42, 3.14f);
    printf("first = %d, second = %f\n", p->first, p->second);
    pair_destroy(p);
}
```

| 宏 | 用途 |
|---|---|
| `pair(T, U)` | 声明一个 pair 结构体类型，包含字段 `first: T` 和 `second: U` |
| `new_pair(T, U, first, second)` | 分配并初始化一个 pair |
| `pair_init(this, first_, second_)` | 初始化一个已分配的 pair |
| `pair_destroy(this)` | 释放堆上分配的 pair |

---

### 9. 私有成员

使用 `private()` 将内部字段隐藏在 opaque 指针之后，通过 `this->priv` 访问私有数据。

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

## 实验性标记

定义于 `flags.h`。编译时使用 `-D` 覆盖。

| 标记 | 默认值 | 作用 |
|---|---|---|
| `EXPER_DELETE` | `1` | `delete(obj)` 无需指定类型——从 `Object` 头推断类型 |
| `EXPER_BIND` | `1` | `bind` 的块外语法：`bind(self, meth, int x) { ... }`（显式参数列表） |

---

## 编译标记参考

| 标记 | 是否必需 | 用途 |
|---|---|---|
| `-fblocks` | **必需** | 启用 Clang Blocks 扩展 |
| `-lBlocksRuntime` | **必需** | 链接 Blocks 运行时库 |
| `-DEXPER_DELETE=0` | 可选 | 禁用无类型 `delete(obj)` |
| `-DEXPER_BIND=0` | 可选 | 禁用扩展 `bind` 语法 |

最小编译命令：

```sh
clang -fblocks -lBlocksRuntime program.c
```

---

## API 参考

### 类定义

| 宏 | 用途 |
|---|---|
| `class(name)` | 声明类结构体，自动添加 `dtor_decl` 与 typedef |
| `interface(name)` | 声明接口（带操作表的抽象类型） |
| `extends(type)` | 继承基类（单继承） |
| `method(ret, name, ...)` | 声明方法（Block 函数指针） |
| `private(cls) { ... }` | opaque 私有数据区 |

### 生命周期

| 宏 | 用途 |
|---|---|
| `ctor(name, ...)` | 定义构造函数；成功时 `return 0` |
| `dtor(name)` | 定义析构函数（释放绑定方法 + 清理） |
| `new(name, ...)` | 在堆上分配并构造对象 |
| `delete(name, obj)` | 调用析构函数并释放（类型安全，`EXPER_DELETE=1` 时可不指定类型） |
| `new(sptr_t, ptr)` | 将堆对象包装为智能指针 |

### 方法绑定

| 宏 | 用途 |
|---|---|
| `bind(self, method) { ... }` | 绑定方法实现（Block）；自动释放之前的绑定 |
| `bind(self, method, ^(...) { ... })` | 使用显式 Block 表达式绑定 |
| `unbind(self, method)` | 释放绑定的方法 |

### 内省

| 宏 | 用途 |
|---|---|
| `instanceof(type, obj)` | 运行时类型检查（`cls_name` 匹配则返回 1） |
| `to_object(obj)` | 从实例指针获取内部 `Object` 头 |

### Blocks 辅助

| 宏 | 用途 |
|---|---|
| `lambda(ret, args, body)` | 内联 lambda 表达式，返回 Block |
| `fptr` | `^` 的简写（Block 指针语法） |
| `lbd_t(ret, args)` | Block 原型类型别名 |

---

## 许可证

MIT — 详见 [LICENSE](./LICENSE)。
