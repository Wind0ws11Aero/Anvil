# Anvil

> 一個健壯、低開銷的 C 語言函式庫，基於 Clang Blocks 擴充，為 C 帶來物件導向程式設計（OOP）、RAII、泛型程式設計、檔案串流以及 C++ 風格的例外處理。

---

## 目錄

- [前置條件](#前置條件)
- [快速開始](#快速開始)
- [特性](#特性)
  - [1. 物件導向程式設計](#1-物件導向程式設計)
  - [2. 介面與繼承](#2-介面與繼承)
  - [3. RAII 智慧指標](#3-raii-智慧指標)
  - [4. Defer](#4-defer)
  - [5. 例外處理](#5-例外處理)
  - [6. 檔案串流](#6-檔案串流)
  - [7. 反射](#7-反射)
  - [8. 泛型程式設計](#8-泛型程式設計)
  - [9. 私有成員](#9-私有成員)
- [實驗性標記](#實驗性標記)
- [編譯標記參考](#編譯標記參考)
- [API 參考](#api-參考)
- [授權條款](#授權條款)

---

## 前置條件

- **Clang**（不支援 GCC — 參見 `oopgcc.h`）
- **Blocks 執行時期**：編譯時加入 `-fblocks`，連結 `-lBlocksRuntime`

```sh
clang -fblocks -lBlocksRuntime program.c
```

---

## 快速開始

```c
#include "oop.h"           // OOP 巨集（自動選擇 clang 後端）
#include "raii.h"          // sptr（智慧指標 / RAII）
#include "defer.h"         // defer（相容 C2Y）
#include "exception.h"     // try / catch / throw
#include "fstream.h"       // 檔案串流，支援鏈式呼叫
#include "reflect.h"       // 執行時期反射
#include "generic/pair.h"  // 泛型 pair<T, U>
```

---

## 特性

### 1. 物件導向程式設計

定義帶有欄位和方法的類別，支援建構子與解構子。每個實例在負偏移量處攜帶 `Object` 標頭，實現執行時期型別檢查與解構分派。

**相關巨集：** `class()`、`method()`、`ctor()`、`dtor()`、`bind()`、`unbind()`、`new()`、`delete()`、`instanceof()`

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
    printf("a 是 A 的實例嗎？%d\n", instanceof(A, a));
    delete(A, a);
}
```

#### 搭配 RAII 智慧指標

```c
void test_raii(void)
{
    sptr a = new(sptr_t, new(A, 1, 2.0f));
    A *obj = a->get_ptr();
    printf("a->a = %d, a->b = %f\n", obj->get_a(), obj->get_b());
    // a 離開作用域時自動釋放
}
```

---

### 2. 介面與繼承

使用 `interface()` 定義抽象契約，透過 `extends()` 實現繼承。方法分派為手動操作表，讓你完全掌控虛擬表佈局。

**相關巨集：** `class()`、`interface()`、`extends()`、`bind()`、`divfn_cast()`

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
    extends(Numeric);       // Integer 繼承 Numeric
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

| 巨集 | 用途 |
|---|---|
| `interface(name)` | 宣告一個帶有操作表的抽象型別 |
| `extends(type)` | 繼承基礎類別的所有欄位 |
| `divfn_cast(base_t, div_t, fn, ...)` | 將基礎類別函式指標轉型為衍生類別函式指標 |

---

### 3. RAII 智慧指標

`sptr` 透過引用計數與 `[[gnu::cleanup]]` 實現自動記憶體管理。當 `sptr` 變數離開作用域時，引用計數遞減——計數歸零時自動釋放所管理的物件。

```c
#include "raii.h"

void example(void)
{
    sptr a = new(sptr_t, new(A, 10, 20.0f));
    sptr b = a->borrow();    // 增加引用計數
    // ... 使用 a->get_ptr() 或 b->get_ptr()
    // b 離開作用域 → 引用計數遞減
    // a 離開作用域 → 引用計數歸零 → 物件釋放
}
```

| 型別 / 巨集 | 用途 |
|---|---|
| `sptr_t` | 智慧指標類別（包含 `sptr_priv`，持有引用計數與原始指標） |
| `sptr` | 帶 `[[gnu::cleanup]]` 自動釋放的變數宣告 |
| `new(sptr_t, ptr)` | 將原始 `new(...)` 指標包裝為智慧指標 |
| `a->borrow()` | 借用引用（引用計數 +1） |
| `a->get_ptr()` | 取出原始指標 |

---

### 4. Defer

`defer` 是 **C2Y** 特性（[ISO/IEC TS 25755](https://www.open-std.org/jtc1/sc22/wg14/)），計劃在下一次 C 標準修訂中正式納入。Anvil 提供了一個**輕量巨集**，其語法與 C2Y 的 `defer` 保持原始碼相容。當編譯器已支援 C2Y（定義了 `__STDC_DEFER_TS25755__`）時，該巨集將直接透傳至 `<stddefer.h>` 中的原生 `_Defer` 實作；否則退回基於 Clang Blocks 與 `[[gnu::cleanup]]` 的實作。

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

    // 使用 arr ...
    for (int i = 0; i < 10; i++)
        arr[i] = i;
    printf("arr[5] = %d\n", arr[5]);
    // main() 返回時 arr 自動釋放
}
```

搭配 `fstream` 使用：

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
    // 離開作用域時檔案自動關閉
}
```

| 實作路徑 | 條件 |
|---|---|
| 原生 C2Y `_Defer` | 已定義 `__STDC_DEFER_TS25755__` |
| Anvil 退回方案 | 使用 `[[gnu::cleanup]]` + Clang Blocks |

---

### 5. 例外處理

基於 `setjmp`/`longjmp` 與執行緒區域例外處理器堆疊實現的 C++ 風格 `try`/`catch`/`throw`。可使用 `throws` 和 `noexcept` 標註巨集來說明函式意圖（它們展開為空）。

**相關巨集：** `try`、`catch`、`throw()`、`throws`、`noexcept`、`exception_as()`、`Exception()`

```c
#include "exception.h"
#include <stdio.h>

int main() throws
{
    try
    {
        throw(Exception("出錯了。\n"));
    }
    catch
    {
        exception_as(e);
        printf("%s", e->msg);
    }
    // 未捕獲的例外將觸發 abort
}
```

| 巨集 | 用途 |
|---|---|
| `throws` | 標註巨集——標記可能拋出例外的函式（展開為空） |
| `noexcept` | 標註巨集——標記不拋出例外的函式（展開為空） |
| `try` | 開啟受保護區塊 |
| `catch` | 捕獲 `try` 區塊中拋出的任何例外 |
| `throw(e)` | 拋出例外（不回傳） |
| `Exception(msg)` | 配置一個包含訊息字串的 `Exception` |
| `exception_as(name)` | 將捕獲的例外繫結到區域變數 |

---

### 6. 檔案串流

支援鏈式呼叫的流式檔案 I/O 封裝。提供 `write`、`flush`、`close`、`rewind`、`seek`、`tell` 以及直接存取底層 `raw_file`。

**相關巨集：** `class()`、`method()`、`ctor()`、`dtor()`、`bind()`、`new()`、`delete()`

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

| 成員 | 型別 | 說明 |
|---|---|---|
| `raw_file` | `FILE *` | 直接存取底層 `FILE *` |
| `write(char *s)` | `fstream *` | 寫入 C 字串；回傳 `this` 以支援鏈式呼叫 |
| `flush(void)` | `fstream *` | 刷新緩衝區；回傳 `this` 以支援鏈式呼叫 |
| `close(void)` | `void` | 關閉底層 `FILE *` |
| `rewind(void)` | `fstream *` | 回到檔案開頭 |
| `seek(long, int)` | `fstream *` | 移動檔案指標到指定偏移量 |
| `tell(void)` | `long` | 取得目前檔案位置 |

---

### 7. 反射

透過編譯期 `offsetof` 與靜態中繼資料註冊表（利用 `__attribute__((constructor))` 自動註冊），實現按名稱存取欄位和呼叫方法。

**相關巨集：** `reflect()`、`rfl_member()`、`getfield()`、`setfield()`、`callmeth()`

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
    printf("%d\n", getfield(A, a, a));      // 按名稱讀取欄位 "a"   → 5
    printf("%d\n", callmeth(A, a, get_a));   // 按名稱呼叫方法 "get_a" → 5
}
```

| 巨集 | 用途 |
|---|---|
| `reflect(cls, ...)` | 為類別註冊反射中繼資料 |
| `rfl_member(cls, member)` | 描述一個成員（名稱 + 偏移量） |
| `getfield(cls, obj, mem)` | 按名稱讀取欄位 |
| `setfield(cls, obj, mem, val)` | 按名稱寫入欄位 |
| `callmeth(cls, obj, mem, ...)` | 按名稱呼叫方法 |

---

### 8. 泛型程式設計

透過 C 巨集實現的型別安全泛型資料結構。包含泛型 `pair<T, U>`。

```c
#include "generic/pair.h"

int main()
{
    pair(int, float) *p = new_pair(int, float, 42, 3.14f);
    printf("first = %d, second = %f\n", p->first, p->second);
    pair_destroy(p);
}
```

| 巨集 | 用途 |
|---|---|
| `pair(T, U)` | 宣告一個 pair 結構體型別，包含欄位 `first: T` 和 `second: U` |
| `new_pair(T, U, first, second)` | 配置並初始化一個 pair |
| `pair_init(this, first_, second_)` | 初始化一個已配置的 pair |
| `pair_destroy(this)` | 釋放堆積上配置的 pair |

---

### 9. 私有成員

使用 `private()` 將內部欄位隱藏在 opaque 指標之後，透過 `this->priv` 存取私有資料。

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

## 實驗性標記

定義於 `flags.h`。編譯時使用 `-D` 覆蓋。

| 標記 | 預設值 | 作用 |
|---|---|---|
| `EXPER_DELETE` | `1` | `delete(obj)` 無需指定型別——從 `Object` 標頭推斷型別 |
| `EXPER_BIND` | `1` | `bind` 的區塊外語法：`bind(self, meth, int x) { ... }`（顯式參數列表） |

---

## 編譯標記參考

| 標記 | 是否必要 | 用途 |
|---|---|---|
| `-fblocks` | **必要** | 啟用 Clang Blocks 擴充 |
| `-lBlocksRuntime` | **必要** | 連結 Blocks 執行時期函式庫 |
| `-DEXPER_DELETE=0` | 可選 | 停用無型別 `delete(obj)` |
| `-DEXPER_BIND=0` | 可選 | 停用擴充 `bind` 語法 |

最小編譯指令：

```sh
clang -fblocks -lBlocksRuntime program.c
```

---

## API 參考

### 類別定義

| 巨集 | 用途 |
|---|---|
| `class(name)` | 宣告類別結構體，自動加入 `dtor_decl` 與 typedef |
| `interface(name)` | 宣告介面（帶操作表的抽象型別） |
| `extends(type)` | 繼承基礎類別（單一繼承） |
| `method(ret, name, ...)` | 宣告方法（Block 函式指標） |
| `private(cls) { ... }` | opaque 私有資料區 |

### 生命週期

| 巨集 | 用途 |
|---|---|
| `ctor(name, ...)` | 定義建構子；成功時 `return 0` |
| `dtor(name)` | 定義解構子（釋放繫結方法 + 清理） |
| `new(name, ...)` | 在堆積上配置並建構物件 |
| `delete(name, obj)` | 呼叫解構子並釋放（型別安全，`EXPER_DELETE=1` 時可不指定型別） |
| `new(sptr_t, ptr)` | 將堆積物件包裝為智慧指標 |

### 方法繫結

| 巨集 | 用途 |
|---|---|
| `bind(self, method) { ... }` | 繫結方法實作（Block）；自動釋放之前的繫結 |
| `bind(self, method, ^(...) { ... })` | 使用顯式 Block 表達式繫結 |
| `unbind(self, method)` | 釋放已繫結的方法 |

### 內省

| 巨集 | 用途 |
|---|---|
| `instanceof(type, obj)` | 執行時期型別檢查（`cls_name` 匹配則回傳 1） |
| `to_object(obj)` | 從實例指標取得內部 `Object` 標頭 |

### Blocks 輔助

| 巨集 | 用途 |
|---|---|
| `lambda(ret, args, body)` | 內聯 lambda 表達式，回傳 Block |
| `fptr` | `^` 的簡寫（Block 指標語法） |
| `lbd_t(ret, args)` | Block 原型型別別名 |

---

## 授權條款

MIT — 詳見 [LICENSE](./LICENSE)。
