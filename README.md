# C 排序算法模块

一组统一的 C 排序接口，覆盖冒泡、鸡尾酒、选择、插入、希尔、归并、快速、堆、计数、基数、桶共 11 种算法，附带自包含测试程序（不依赖第三方框架）。

## 目录结构

```text
c-sorting-algorithms/
├── include/
│   └── sort.h              # 公开接口：状态枚举、11 个排序函数、算法元信息
├── src/
│   ├── sort_internal.h     # 内部共享声明（参数校验、区间插入排序、交换）
│   ├── sort_support.c      # 参数校验、状态字符串、插入排序辅助
│   ├── sort_registry.c     # 算法注册表与复杂度元信息
│   ├── sort_bubble.c
│   ├── sort_cocktail.c
│   ├── sort_selection.c
│   ├── sort_insertion.c
│   ├── sort_shell.c
│   ├── sort_merge.c
│   ├── sort_quick.c
│   ├── sort_heap.c
│   ├── sort_counting.c
│   ├── sort_radix.c
│   └── sort_bucket.c
├── tests/
│   └── test_sort.c         # 测试套件，结果与 qsort() 对比
├── Makefile                # POSIX shell（Linux / macOS / WSL）
└── build.ps1               # Windows PowerShell 构建脚本
```

## 构建与测试

Windows（PowerShell 5.1+，需要 `gcc` 在 `PATH` 中）：

```powershell
.\build.ps1              # Release 构建并运行测试
.\build.ps1 -Clean       # 清理后重建
.\build.ps1 -Configuration Debug
```

Linux / macOS / WSL：

```sh
make test
make clean
```

编译选项默认 `-std=c11 -O2 -Wall -Wextra -Wpedantic`，测试通过时退出码为 0。

## 接口约定

```c
typedef enum {
    SORT_OK = 0,
    SORT_INVALID_ARGUMENT,
    SORT_OUT_OF_MEMORY,
    SORT_RANGE_ERROR
} SortStatus;

SortStatus sort_bubble(int *array, size_t length);
```

所有 `sort_*` 函数遵循同一套约定：

| 情形 | 行为 |
| --- | --- |
| `length == 0`，`array == NULL` | 返回 `SORT_OK` |
| `length > 0`，`array == NULL` | 返回 `SORT_INVALID_ARGUMENT` |
| `length < 2` | 返回 `SORT_OK`，不修改数组 |
| 动态内存分配失败 | 返回 `SORT_OUT_OF_MEMORY` |
| 值域超出算法限制（计数排序） | 返回 `SORT_RANGE_ERROR` |
| 任意非 `SORT_OK` 返回 | 数组内容保持不变 |

- 元素类型为 `int`，元素个数使用 `size_t`。
- 头文件已包含 `<stddef.h>`，使用 include guard，可直接用于 C 和 C++ 项目（`extern "C"`）。
- `sort_status_string()` 返回状态名称，未知状态返回 `"unknown SortStatus"`，不会返回 `NULL`。
- 算法元信息可通过注册表读取：`sort_algorithm_count()`、`sort_algorithm_at(index)`、`sort_algorithm_find("merge")`；未知名称或越界索引返回 `NULL`。

### 可调限制

| 宏 | 默认值 | 含义 |
| --- | --- | --- |
| `SORT_COUNTING_MAX_RANGE` | `4194304` | 计数排序允许的 `max - min + 1` 上限；超出即返回 `SORT_RANGE_ERROR`，避免无界分配 |
| `SORT_BUCKET_MAX_COUNT` | `65536` | 桶排序创建的桶数量上限（同时不超过元素个数） |

可在编译时覆盖，例如 `-DSORT_COUNTING_MAX_RANGE=1000000`。

## 算法特性

`k` 为值域大小，`d` 为 `int` 的字节数，`b = 256`。

| 算法 | 函数 | 最好 | 平均 | 最坏 | 额外空间 | 稳定 | 原地 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 冒泡 | `sort_bubble` | n | n² | n² | 1 | 是 | 是 |
| 鸡尾酒 | `sort_cocktail` | n | n² | n² | 1 | 是 | 是 |
| 选择 | `sort_selection` | n² | n² | n² | 1 | 否 | 是 |
| 插入 | `sort_insertion` | n | n² | n² | 1 | 是 | 是 |
| 希尔 | `sort_shell` | n log n | n^1.25（经验） | n^1.5 | 1 | 否 | 是 |
| 归并 | `sort_merge` | n log n | n log n | n log n | n | 是 | 否 |
| 快速 | `sort_quick` | n log n | n log n | n log n | log n | 否 | 是 |
| 堆 | `sort_heap` | n log n | n log n | n log n | 1 | 否 | 是 |
| 计数 | `sort_counting` | n + k | n + k | n + k | n + k | 是 | 否 |
| 基数 | `sort_radix` | d(n + b) | d(n + b) | d(n + b) | n + b | 是 | 否 |
| 桶 | `sort_bucket` | n + k | n + k（均匀分布） | n²（数据聚簇） | n + k | 是 | 否 |

实现要点：

- **快速排序**：三路划分（重复元素友好）+ ninther（三数取中再取中）+ 小区间插入排序；始终先处理较小的一侧、把较大的一侧压入显式栈，栈深度不超过 O(log n)；每个区间携带 `2 * log2(n)` 的深度预算，预算耗尽即改用堆排序收尾，因此最坏情况下仍是 O(n log n)。
- **归并排序**：自底向上，只分配一块 `n` 大小的临时缓冲区；相邻 run 已有序时跳过合并。
- **基数排序**：对 `(unsigned int)value ^ 符号位` 做 LSD 分位排序，把有符号顺序映射为无符号顺序，因此 `INT_MIN`、`INT_MAX` 和负数都正确，且不产生有符号溢出。
- **计数排序**：先校验值域再分配；用前缀偏移正向散射，保证稳定性。
- **桶排序**：按值域均匀分桶，正向散射保持相对顺序，桶内用稳定插入排序收尾，故整体稳定。
- **希尔排序**：Knuth 的 3k + 1 增量序列，最坏 O(n^1.5)。

## 验证

`tests/test_sort.c` 对 11 种算法跑同一组用例，所有结果都与 `qsort()` 的参考结果逐元素比较（同时验证“有序”和“是输入的排列”）：

- 空输入：`(NULL, 0)` 与 `(合法指针, 0)`；
- 错误输入：`(NULL, 3)` 必须返回 `SORT_INVALID_ARGUMENT`；
- 单元素、两元素（有序/逆序）、已升序、已降序、全部相等；
- 重复元素、负数、`INT_MIN` / `INT_MAX` 边界；
- 规模化用例：升序、降序、成对重复、锯齿、二值、全等、宽范围随机、窄范围随机、全 `int` 范围随机；
- 计数排序在值域超限时返回 `SORT_RANGE_ERROR`，并检查数组未被修改；
- 每次调用都在缓冲区前后放置哨兵值，检测越界写入。

每个算法的压力规模：二次复杂度算法 1000 个元素，希尔 4000，其余 20000。

运行结果（MinGW.org GCC 6.3.0，`-std=c11 -O2 -Wall -Wextra -Wpedantic`，无警告；另用 `-Werror -Wconversion -Wshadow -Wstrict-prototypes` 复核通过）：

```text
284 checks, 0 failure(s)
ALL TESTS PASSED
```

## 已知限制

- **稳定性无法通过当前 API 观测**：只有 `int` 元素，相等元素不可区分。稳定性结论来自实现方式（稳定的散射/合并/插入过程），未用带下标的元素做端到端验证。如需在测试中验证稳定性，应增加带 `size_t` 下标的泛型接口版本。
- **计数排序值域受限**：默认值域上限 4,194,304；超出返回 `SORT_RANGE_ERROR`，需要时由调用方改用基数排序或自行放宽宏。
- **桶排序最坏情况 O(n²)**：所有元素落入同一个桶且桶内逆序时；均匀分布时接近 O(n)。
- **仅支持 `int` 数组**：未提供泛型（`void *` + 元素大小 + 比较函数）版本，也未提供结构体或其他数据类型。
- **不包含基准测试**：只验证正确性与边界行为，不测量实际耗时。
- **未验证 16 / 64 位 `int` 平台**：基数排序按 `sizeof(int) * CHAR_BIT` 计算分位趟数，逻辑上可移植，但实测环境只有 32 位 `int`。`size_t` 小于 `int` 的极端平台未测试。
