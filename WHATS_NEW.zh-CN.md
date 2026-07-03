# Anvil 2.0 更新内容

Anvil 2.0 是一次重大发布，引入了新的语言级构造，重构了 OOP 体系，并全面提升了开发体验。本文档涵盖了从 1.x 到 2.0 的关键变化。

---

## 破坏性变更

### `class()` 与 `interface()` — OOP 体系分离

在 1.x 中，`class()` 的行为类似接口——它声明一个带 typedef 的结构体，但不附加构造/析构绑定代码。在 2.0 中，`class()` 现在自动生成析构函数声明（`dtor_decl`），并期望完整的 OOP 生命周期。如果只需要一个裸结构体加 typedef，请使用新增的 `interface()` 宏。

**1.x：**
```c
class (A)
{
    int a;
    method(int, get_a, void);
};
```

**2.0：**
```c
// 完整 OOP 类，具有构造/析构生命周期
class (A)
{
    int a;
    method(int, get_a, void);
};

// 裸抽象类型（即旧版 class 的行为）
interface (Numeric)
{
    struct Numeric_ops
    {
        method(Numeric *, add, Numeric *);
    } ops;
};
```

### `private()` 语法变更

`private()` 现在使用花括号块而非可变参数。访问方式仍通过 `this->priv`。

**1.x：**
```c
private(A, int a; float b;);
```

**2.0：**
```c
private(A)
{
    int a;
    float b;
};
```

### `Object` 头重新设计

内部元数据头已从 `object_t` 升级为 `Object`。每个通过 `new()` 创建的对象现在携带包含 `cls_name` 和 `dtor_fn` 的完整 `Object` 头，用于运行时类型内省和自动析构分发。`struct object_t` 的 typedef 已移除——请使用 `Object`。

### 移除 GCC 支持

Anvil 现在仅面向 **Clang**。`oopgcc.h` 后端已移除。Blocks 扩展和 `__attribute__((cleanup))` 是 Anvil 设计的基础，而这些是 Clang 原生特性。

### 异常系统重构

`Exception` 和 `ExceptionHandler` 已重写为正式的 OOP 类（使用 `class()`、`ctor()`、`bind()`）。旧的扁平结构体方案已废弃。

**1.x：**
```c
typedef struct Exception { /* 原始结构体 */ } Exception;
thread_local ExceptionHandler _handler;
```

**2.0：**
```c
class (Exception) { /* 完整 OOP 类 */ };
class (ExceptionHandler) { /* 带 push/pop/peek 方法的正式类 */ };
```

---

## 新增特性

### Defer — 兼容 C2Y

`defer` 将 Go 风格的延迟执行引入 C，与即将到来的 C2Y 标准（ISO/IEC TS 25755）保持源码兼容。当编译器定义了 `__STDC_DEFER_TS25755__` 时，该宏将直接透传至 `<stddefer.h>` 的原生 `_Defer`；否则回退到基于 Clang Blocks 的实现。

```c
#include "defer.h"

int main(void)
{
    int *arr = malloc(sizeof(int) * 10);
    defer
    {
        free(arr);
    };
    // main() 返回时 arr 自动释放
}
```

### 接口与继承

新增的 `interface()` 宏用于声明带操作表的抽象类型，搭配 `extends()` 实现单继承。方法分派为手动控制，让你完全掌控虚表布局。新的 `divfn_cast()` 可安全地将基类函数指针转型为派生类函数指针。

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
    extends(Numeric);        // Integer 继承 Numeric 接口
    int v;
    method(Integer *, add, Integer *);
};
```

### 无类型 `delete()` — 实验性

当 `EXPER_DELETE=1`（2.0 默认值）时，可直接调用 `delete(obj)` 而无需指定类型。类型将从每个实例附带的 `Object` 头中推断。

```c
A *a = new(A, 1, 2.0f);
delete(a);          // 2.0：无需类型参数
// delete(A, a);    // 1.x：需要指定类型
```

### 扩展 `bind()` 语法 — 实验性

当 `EXPER_BIND=1`（2.0 默认值）时，`bind()` 支持块外参数列表语法，让你在绑定时直接看到方法签名：

```c
bind(this, write, char *cont)
{
    if (fputs(cont, this->raw_file) < 0) throw(Exception("错误"));
    return this;
};
```

### 特性标记系统

新的 `flags.h` 头文件为实验性功能提供编译期开关：

| 标记 | 默认值 | 用途 |
|---|---|---|
| `EXPER_DELETE` | `1` | 无类型 `delete(obj)` |
| `EXPER_BIND` | `1` | 带显式参数的扩展 `bind` 语法 |

使用 `-D` 覆盖：
```sh
clang -fblocks -lBlocksRuntime -DEXPER_BIND=0 program.c
```

### `throws` / `noexcept` 标注

新增用于文档化异常保证的标注宏。两者均展开为空——它们作为可读性提示，也为未来的静态分析预留空间。

```c
int main(void) throws
{
    // 可能抛出异常
}

int safe_fn(void) noexcept
{
    // 绝不抛出异常
}
```

### Blocks 辅助宏

新的 Clang Blocks 实用宏：

| 宏 | 用途 |
|---|---|
| `lambda(ret, args, body)` | 内联 lambda 表达式，返回 Block |
| `fptr` | `^` 的简写（Block 指针） |
| `lbd_t(ret, args)` | Block 原型类型别名 |

```c
lbd_t(int, (int, int)) add = ^int(int a, int b) { return a + b; };
auto predicate = lambda(bool, (int x), { return x > 0; });
```

---

## 迁移指南

### 从 1.x 迁移到 2.0

1. **将 `object_t` 替换为 `Object`**，如果你曾直接使用它。

2. **将可变参数 `private()` 替换为花括号 `private()`：**
   ```c
   // 之前
   private(A, int x; float y;);
   // 之后
   private(A) { int x; float y; };
   ```

3. **如果你的 `class()` 只用于裸结构体，改用 `interface()`：**
   ```c
   // 之前
   class (Shape) { void (*draw)(void); };
   // 之后
   interface (Shape) { void (*draw)(void); };
   ```

4. **为之前没有析构函数的类添加 `dtor()`。**`class()` 现在期望存在析构函数。

5. **直接操作原始结构体的异常代码**需要更新为新的基于 OOP 的 `Exception` 类。

6. **如果无类型 `delete` 导致编译问题**，添加 `-DEXPER_DELETE=0`（绝大多数项目不受影响）。

7. **如果新的块外 bind 语法与现有宏冲突**，添加 `-DEXPER_BIND=0`。

---

## 完整变更日志

### 新增
- `defer.h` — 兼容 C2Y 的 `defer` 宏
- `flags.h` — 特性标记系统（`EXPER_DELETE`、`EXPER_BIND`）
- `interface()` 宏 — 带操作表的抽象类型声明
- `extends()` 宏 — 单继承
- `divfn_cast()` — 基类到派生类的函数指针转型
- `throws` / `noexcept` — 异常标注宏
- `lambda()`、`fptr`、`lbd_t` — Blocks 实用宏
- 扩展 `bind()` 语法，支持显式参数列表
- 通过运行时类型推断实现的无类型 `delete()`
- Object 头现在存储 `dtor_fn` 以支持自动析构分发
- `EXPER_DELETE=1` 时 `delete()` 从 Object 头推断类型

### 变更
- `class()` 现在自动声明析构函数（`dtor_decl`），与 `interface()` 区分开来
- `private()` 从可变参数语法改为花括号块语法
- `object_t` → `Object`（头结构体重命名）
- `Exception` 和 `ExceptionHandler` 重构为 OOP 类
- `new()` 附加包含 `cls_name` + `dtor_fn` 的完整 `Object` 头

### 移除
- GCC 支持（`oopgcc.h` 后端）
- `object_t` typedef（替换为 `Object`）

### 修复
- 析构函数声明问题（`dtor_decl` 的 extern 修复）
- 方法绑定内存安全（自动 `Block_release` 之前的绑定）
- `sptr_t` 中的引用计数竞态条件（切换为 `atomic_int`）
