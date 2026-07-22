# Anvil 2.0 更新內容

Anvil 2.0 是一次重大發布，引入了新的語言級構造，重構了 OOP 體系，並全面提升了開發體驗。本文件涵蓋了從 1.x 到 2.0 的關鍵變化。

---

## 破壞性變更

### `class()` 與 `interface()` — OOP 體系分離

在 1.x 中，`class()` 的行為類似介面——它宣告一個帶 typedef 的結構體，但不附加建構/解構繫結程式碼。在 2.0 中，`class()` 現在自動生成解構子宣告（`dtor_decl`），並期望完整的 OOP 生命週期。如果只需要一個裸結構體加 typedef，請使用新增的 `interface()` 巨集。

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
// 完整 OOP 類別，具有建構/解構生命週期
class (A)
{
    int a;
    method(int, get_a, void);
};

// 裸抽象型別（即舊版 class 的行為）
interface (Numeric)
{
    struct Numeric_ops
    {
        method(Numeric *, add, Numeric *);
    } ops;
};
```

### `private()` 語法變更

`private()` 現在使用花括號區塊而非可變參數。存取方式仍透過 `this->priv`。

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

### `Object` 標頭重新設計

內部中繼資料標頭已從 `object_t` 升級為 `Object`。每個透過 `new()` 建立的物件現在攜帶包含 `cls_name` 和 `dtor_fn` 的完整 `Object` 標頭，用於執行時期型別內省和自動解構分派。`struct object_t` 的 typedef 已移除——請使用 `Object`。

### 移除 GCC 支援

Anvil 現在僅面向 **Clang**。`oopgcc.h` 後端已移除。Blocks 擴充是 Anvil 設計的基礎，而這些是 Clang 原生特性。

### 例外系統重構

`Exception` 和 `ExceptionHandler` 已重寫為正式的 OOP 類別（使用 `class()`、`ctor()`、`bind()`）。舊的扁平結構體方案已廢棄。

**1.x：**
```c
typedef struct Exception { /* 原始結構體 */ } Exception;
thread_local ExceptionHandler _handler;
```

**2.0：**
```c
class (Exception) { /* 完整 OOP 類別 */ };
class (ExceptionHandler) { /* 帶 push/pop/peek 方法的正式類別 */ };
```

---

## 新增特性

### Defer — 相容 C2Y

`defer` 將 Go 風格的延遲執行引入 C，與即將到來的 C2Y 標準（ISO/IEC TS 25755）保持原始碼相容。當編譯器定義了 `__STDC_DEFER_TS25755__` 時，該巨集將直接透傳至 `<stddefer.h>` 的原生 `_Defer`；否則退回基於 Clang Blocks 的實作。

```c
#include "defer.h"

int main()
{
    int *arr = malloc(sizeof(int) * 10);
    defer
    {
        free(arr);
    };
    // main() 返回時 arr 自動釋放
}
```

### 介面與繼承

新增的 `interface()` 巨集用於宣告帶操作表的抽象型別，搭配 `extends()` 實現單一繼承。方法分派為手動控制，讓你完全掌控虛擬表佈局。新的 `divfn_cast()` 可安全地將基礎類別函式指標轉型為衍生類別函式指標。

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
    extends(Numeric);        // Integer 繼承 Numeric 介面
    int v;
    method(Integer *, add, Integer *);
};
```

### 無型別 `delete()` — 實驗性

當 `EXPER_DELETE=1`（2.0 預設值）時，可直接呼叫 `delete(obj)` 而無需指定型別。型別將從每個實例附帶的 `Object` 標頭中推斷。

```c
A *a = new(A, 1, 2.0f);
delete(a);          // 2.0：無需型別參數
// delete(A, a);    // 1.x：需要指定型別
```

### 擴充 `bind()` 語法 — 實驗性

當 `EXPER_BIND=1`（2.0 預設值）時，`bind()` 支援區塊外參數列表語法，讓你在繫結時直接看到方法簽章：

```c
bind(this, write, char *cont)
{
    if (fputs(cont, this->raw_file) < 0) throw(Exception("錯誤"));
    return this;
};
```

### 特性標記系統

新的 `flags.h` 標頭檔為實驗性功能提供編譯期開關：

| 標記 | 預設值 | 用途 |
|---|---|---|
| `EXPER_DELETE` | `1` | 無型別 `delete(obj)` |
| `EXPER_BIND` | `1` | 帶顯式參數的擴充 `bind` 語法 |

使用 `-D` 覆蓋：
```sh
clang -fblocks -lBlocksRuntime -DEXPER_BIND=0 program.c
```

### `throws` / `noexcept` 標註

新增用於文件化例外保證的標註巨集。兩者均展開為空——它們作為可讀性提示，也為未來的靜態分析預留空間。

```c
int main() throws
{
    // 可能拋出例外
}

int safe_fn(void) noexcept
{
    // 絕不拋出例外
}
```

### Blocks 輔助巨集

新的 Clang Blocks 實用巨集：

| 巨集 | 用途 |
|---|---|
| `lambda(ret, args, body)` | 內聯 lambda 表達式，回傳 Block |
| `fptr` | `^` 的簡寫（Block 指標） |
| `lbd_t(ret, args)` | Block 原型型別別名 |

```c
lbd_t(int, (int, int)) add = ^int(int a, int b) { return a + b; };
auto predicate = lambda(bool, (int x), { return x > 0; });
```

---

## 遷移指南

### 從 1.x 遷移到 2.0

1. **將 `object_t` 替換為 `Object`**，如果你曾直接使用它。

2. **將可變參數 `private()` 替換為花括號 `private()`：**
   ```c
   // 之前
   private(A, int x; float y;);
   // 之後
   private(A) { int x; float y; };
   ```

3. **如果你的 `class()` 只用於裸結構體，改用 `interface()`：**
   ```c
   // 之前
   class (Shape) { void (*draw)(void); };
   // 之後
   interface (Shape) { void (*draw)(void); };
   ```

4. **為之前沒有解構子的類別加入 `dtor()`。**`class()` 現在期望存在解構子。

5. **直接操作原始結構體的例外程式碼**需要更新為新的基於 OOP 的 `Exception` 類別。

6. **如果無型別 `delete` 導致編譯問題**，加入 `-DEXPER_DELETE=0`（絕大多數專案不受影響）。

7. **如果新的區塊外 bind 語法與現有巨集衝突**，加入 `-DEXPER_BIND=0`。

---

## 完整變更日誌

### 新增
- `defer.h` — 相容 C2Y 的 `defer` 巨集
- `flags.h` — 特性標記系統（`EXPER_DELETE`、`EXPER_BIND`）
- `interface()` 巨集 — 帶操作表的抽象型別宣告
- `extends()` 巨集 — 單一繼承
- `divfn_cast()` — 基礎類別到衍生類別的函式指標轉型
- `throws` / `noexcept` — 例外標註巨集
- `lambda()`、`fptr`、`lbd_t` — Blocks 實用巨集
- 擴充 `bind()` 語法，支援顯式參數列表
- 透過執行時期型別推斷實現的無型別 `delete()`
- Object 標頭現在儲存 `dtor_fn` 以支援自動解構分派
- `EXPER_DELETE=1` 時 `delete()` 從 Object 標頭推斷型別

### 變更
- `class()` 現在自動宣告解構子（`dtor_decl`），與 `interface()` 區分開來
- `private()` 從可變參數語法改為花括號區塊語法
- `object_t` → `Object`（標頭結構體重新命名）
- `Exception` 和 `ExceptionHandler` 重構為 OOP 類別
- `new()` 附加包含 `cls_name` + `dtor_fn` 的完整 `Object` 標頭

### 移除
- GCC 支援（`oopgcc.h` 後端）
- `object_t` typedef（替換為 `Object`）

### 修復
- 解構子宣告問題（`dtor_decl` 的 extern 修復）
- 方法繫結記憶體安全（自動 `Block_release` 之前的繫結）
- `sptr_t` 中的引用計數競態條件（切換為 `atomic_int`）
