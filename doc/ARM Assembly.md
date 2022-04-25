# GUN 汇编语法

对于ARM架构下的汇编语言，编译使用的是gcc交叉编译工具链，汇编代码要符合GNU汇编语法。

GNU汇编语法适用于所有的架构，并不是ARM独享的，GNU汇编由一系列的语句组成，每行一条语句，每条语句有3个可选部分，如下所示：

```
label:		instruction|directive|pseudo-instruction		@comments
```

其中：

- `label` 是标号
  - 标号的本质是代表它所在的地址，因此也可以当作变量或者函数来使用
  - 汇编程序的缺省入口是 `_start` 标号，用户也可以在连接脚本文件中用 `ENTRY` 标志指明其它入口点
- `instruction` 是指令
- `directive` 是伪操作
- `pseudo-instruction` 是伪指令
- `comments` 是注释

ARM指令，伪指令，伪操作，寄存器名可以全部为大写字母，也可全部为小写字母，但不可大小写混用。

汇编系统预定义了几个段名：

- `.test`：代码段
- `.data`：初始化数据段
- `.bss`：未初始化数据段，应该在代码段之前



# GNU ARM汇编的常用伪操作

数据定义伪操作：

- 整数：`.byte` 单字节，`.short` 双字节，`.long` 四字节，`.quad` 八字节
- 浮点数：`.float`
- 字符串：`.string`，`.ascii`（需自己添加 `\0`）

其他伪操作：

- `.global symbol`：定义一个全局符号，以便其他文件中使用符号
- `.type main, %function`：说明 `main` 是一个函数



# ARM 寄存器和函数调用

**General Purpose Registers**

|   #   | Alias | Purpose              |
| :---: | :---: | -------------------- |
| `r0`  |   -   | General purpose      |
| `r1`  |   -   | General purpose      |
| `r2`  |   -   | General purpose      |
| `r3`  |   -   | General purpose      |
| `r4`  |   -   | General purpose      |
| `r5`  |   -   | General purpose      |
| `r6`  |   -   | General purpose      |
| `r7`  |   -   | Holds Syscall Number |
| `r8`  |   -   | General purpose      |
| `r9`  |   -   | General purpose      |
| `r10` |   -   | General purpose      |
| `r11` | `fp`  | Frame Pointer        |

**Special Purpose Registers**

|   #    | Alias | Purpose                         |
| :----: | :---: | ------------------------------- |
| `r12`  | `ip`  | Intra Procedural Call           |
| `r13`  | `sp`  | Stack Pointer                   |
| `r14`  | `lr`  | Link Register                   |
| `r15`  | `pc`  | Program Counter                 |
| `cpsr` |   -   | Current Program Status Register |

函数调用规则（暂时没有找到相关标准文档，基于 `arm-linux-gnueabihf-gcc` 的结果进行修改，不一定准确，仅适用于本项目）

<img src="https://neatlii-markdown.oss-cn-shanghai.aliyuncs.com/MarkdownImage/image-20220328204302995.png" alt="image-20220328204302995" style="zoom:80%;" />



# ARM 常用指令

### 符号说明

- `#<imm16>`，`#<imm32>`：16位和32位立即数
- `#<imm8m>`：
  - 由于某些指令只有12-bit来表示立即数，因此采用 `"#<imm8> ror (#<imm4> * 2)"` 的方式，以扩大表示范围
  - 不是所有立即数都合法，建议对不合法的立即数使用 `ldr` 伪指令
- `#<offset>`：寻址偏移量有一定限制，目前暂时不用管
- `<reglist>`：由逗号分隔，被一对大括号包围的寄存器列表
- `{cond}`：
  - 依据不同的条件，判断是否实行该指令。通过 `cmp` 指令修改 `cpsr`
  - 有 `eq, ne, gt, ge, lt, le` 六种条件

### `mov`

```
mov{cond} Rd, Rm
mov{cond} Rd, #<imm16>
mov{cond} Rd, #<imm8m>
```

### `LDR`

```
ldr{cond} Rd, [Rn]
ldr{cond} Rd, [Rn, #<offset>]
ldr{cond} Rd, =#<imm32>			@ pseudo-instruction
```

### `STR`

```
str{cond} Rd, [Rn]
str{cond} Rd, [Rn, #<offset>]
```

### `PUSH`

```
push{cond} <reglist>
```

### `POP`

```
pop{cond} <reglist>
```

### `CMP`

```
cmp{cond} Rn, Rm
cmp{cond}, Rn, #<imm8m>
```

### `b`

```
b{cond} label		@ 直接跳转
```

### `bl`

```
bl{cond} label		@ 保存pc到lr，然后再跳转
```

### `bx`

```
bx{cond} Rm			@ 和bl配合，通过bx lr实现函数返回
```

### `add`

```
add{cond} Rd, Rn, Rm
add{cond} Rd, Rn, #<imm8m>
```

### `sub`

```
sub{cond} Rd, Rn, Rm
sub{cond} Rd, Rn, #<imm8m>
```

### `mul`

```
mul{cond} Rd, Rm, Rs
```

### `sdiv`

```
sdiv{cond} Rd, Rn, Rm
```

### `and`

```
and{cond} Rd, Rn, Rm
and{cond} Rd, Rn, #<imm8m>
```

### `orr`

```
orr{cond} Rd, Rn, Rm
orr{cond} Rd, Rn, #<imm8m>
```

### `nop`

```
nop{cond}	@ pseudo-instruction
```

