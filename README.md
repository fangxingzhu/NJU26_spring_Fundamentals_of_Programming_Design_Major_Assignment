# STS IO 库

用于命令行程序的简单 I/O 库。

## 快速开始

在你的 `.c` 文件中包含头文件：

```c
#include "sts_io.h"
```

然后将 `sts_io.h` 和 `sts_io.c` 添加到你的项目中，按照你所使用的 IDE 或编译环境的方式进行编译。

---

## 功能列表

### 安全输入

| 函数 | 说明 |
|------|------|
| `sts_read_int(prompt)` | 读取一个整数，自动验证输入格式 |
| `sts_read_int_range(prompt, min, max)` | 读取指定范围内的整数 |
| `sts_read_char(prompt)` | 读取单个字符，验证只输入了一个字符 |
| `sts_read_bool(prompt)` | 读取 yes/no 回答（仅接受 y/n/yes/no） |
| `sts_read_line(prompt, buf, size)` | 读取一行文本 |
| `sts_pause(prompt)` | 等待用户按回车，`prompt` 为 `NULL` 时输出默认提示词 |

**示例：**

```c
int age = sts_read_int("请输入年龄: ");
int choice = sts_read_int_range("请选择 (1-4): ", 1, 4);
if (sts_read_bool("继续吗? (y/n): ")) {
    // 用户输入了 yes 或 y
}
sts_pause(NULL); // 使用 NULL 需要引入 stddef.h 或 stdlib.h
```

### 颜色支持

| 函数 | 说明 |
|------|------|
| `sts_set_color(color)` | 设置后续输出的文本颜色 |
| `sts_reset_color()` | 恢复默认颜色 |

可用颜色：
- `STS_COLOR_RED` - 红色
- `STS_COLOR_GREEN` - 绿色
- `STS_COLOR_BLUE` - 蓝色
- `STS_COLOR_YELLOW` - 黄色
- `STS_COLOR_CYAN` - 青色
- `STS_COLOR_MAGENTA` - 洋红
- `STS_COLOR_WHITE` - 白色

以及对应的亮色版本（如 `STS_COLOR_BRIGHT_RED`）。

**示例：**

```c
sts_set_color(STS_COLOR_RED);
sts_print("你受到了 5 点伤害！\n");
sts_reset_color();

sts_set_color(STS_COLOR_GREEN);
sts_print("你恢复了 10 点生命！\n");
sts_reset_color();
```

### 输出与屏幕控制

| 函数 | 说明 |
|------|------|
| `sts_print(...)` | 类型安全的多参数打印（支持 1-10 个参数） |
| `sts_println(...)` | 类型安全的多参数打印（自动换行，支持 1-10 个参数） |
| `sts_printf(format, ...)` | 类似 printf 的格式化输出 |
| `sts_puts(line)` | 输出一行（自动添加换行） |
| `sts_separator()` | 输出 80 字符宽的分隔线 |
| `sts_title(title)` | 输出居中的标题（带分隔线） |
| `sts_clear_screen()` | 清屏（跨平台） |

**类型安全打印示例：**

```c
int age = 25;
const char* name = "Alice";
double hp = 70.5;
long long gold = 999999;
unsigned int score = 1000;

/* 多参数打印，无需格式化字符串 */
sts_print("Name: ", name, ", Age: ", age, "\n");

/* 自动换行版本 */
sts_println("HP: ", hp, "/", 70);

/* 混合使用不同类型（支持最多 10 个参数） */
sts_println("Player: ", name,
            " (", age, "), ",
            "HP: ", hp, "/", 70,
            ", Gold: ", gold);
```

`sts_print` 和 `sts_println` 支持以下类型：
- 字符串：`char*`, `const char*`
- 整数：`int`, `long`, `long long`
- 无符号整数：`unsigned int`, `unsigned long`, `unsigned long long`
- 浮点数：`float`, `double`
- 字符：`char`
- 布尔值：`bool`, `_Bool` (打印 "true" 或 "false")

### HUD 回调

**前置知识：函数指针**

| 函数 | 说明 |
|------|------|
| `sts_set_header(func)` | 设置头部回调函数，每次清屏后自动调用 |
| `sts_clear_header()` | 清除头部回调 |

**示例：**

```c
/* 定义一个回调函数来显示 HUD */
void draw_hud(void) {
    sts_print("HP: ", player_hp, "/", player_max_hp, "  ");
    sts_print("Gold: ", gold, "\n");
    sts_separator();
}

/* 设置回调，只需设置一次 */
sts_set_header(draw_hud);

/* 之后每次清屏都会自动显示 HUD */
sts_clear_screen();
```

---

## 更多示例

### 完整输入输出流程

```c
#include "sts_io.h"

int main(void) {
    char name[256];

    sts_title("My Game");
    sts_puts("");

    sts_read_line("你的名字: ", name, sizeof(name));
    int age = sts_read_int("你的年龄: ");

    sts_puts("");
    sts_println("你好, ", name, "!");

    if (sts_read_bool("开始游戏? (y/n): ")) {
        sts_set_color(STS_COLOR_GREEN);
        sts_puts("游戏开始！");
        sts_reset_color();
    }

    return 0;
}
```

### 使用颜色和标题

```c
sts_clear_screen();
sts_title("Battle!");
sts_puts("");

sts_set_color(STS_COLOR_BRIGHT_WHITE);
sts_puts("Player:");
sts_reset_color();

sts_set_color(STS_COLOR_GREEN);
sts_print("  HP: ", player_hp, "/", player_max_hp);
sts_reset_color();
```
