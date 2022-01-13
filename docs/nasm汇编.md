# nask 汇编

* Notice
> 可以间接寻址的寄存器: `BX`, `BP`, `SI`, `DI`
> 系统启动内存地址:     `0x7c00`
> 第一扇区结尾符号:     `0x55aa`

## 通用寄存器

16bit | \
------| ----------------
AX    | accumulator 累加寄存器
CX    | counter 计数寄存器
DX    | data 数据寄存器
BX    | base 基地址寄存器
SP    | stack 栈顶地址寄存器
BP    | base 栈指针寄存器
SI    | source 源地址寄存器
DI    | destination 目的地址寄存器

## 段寄存器

> 段寄存器不能直接赋值，需要使用`MOV ES, AX`进行赋值
> 段寄存器不能进行加减运算！
16bit | \
------| ----------------
ES    | extra segment 附加段寄存器
CS    | code segment 代码段寄存器
SS    | stack segment 栈段寄存器
DS    | data segment 数据段寄存器，`也是默认段寄存器`，默认为0。
FS    | 没有名称
GS    | 没有名称

## 标志寄存器

**只能用pushfl和popfl操作标志寄存器**

## 实模式下的汇编

### ORG 0x7c00

> 指示编译器代码将加载到0x7c00

[org 0x7c00解疑](https://blog.csdn.net/judyge/article/details/52333656)

### $

> 当前指令地址

### JC

> 如果寄存器 CF 为 1, 则跳转到指定地址 jump if carry

```S
JC error
```

### JBE JE

和CMP指令协同使用。

> JBE 小于或等于 jump if below or equal
> JE 等于 jump if equal

### INT

> 异常中断


## 保护模式下的汇编

### GDT

[GDT结构](./段描述符结构.png)

1. 为了表示一个段，需要一下信息（64位）：

    - 段的大小
    - 段的起始地址
    - 段的管理属性
    - 段描述符结构体如下：
        ```C
        struct gdt_entry {
            uint16_t limit_low;     // 段长度 0-15  位
            uint16_t base_low;      // 段基址 0-15  位
            uint8_t base_middle;    // 段基址 16-23 位
            uint8_t access;         // 段的访问权属性
            uint8_t access_and_limit_high;     // 段的访问权属性（高4位） 段长度 16-19 位（低4位）
            uint8_t base_high;      // 段基址 24-31 位
        } __attribute__((packed));
        ```
    
    > 段长度只有20位，只能表示1M的内存空间。为了表示4G的内存空间，可以设置段描述符的第23位为1， 此时段长度的单位为页（4KB）。1M的内存空间可以表示1073741824页，`4G = 4KB * 1073741824`页。

    - `access_and_limit_high`的高4位：

        bit | 作用
        --- | -
        G   | 用于指示段大小的单位，0表示1B，1表示4K
        D   | 用于指示段的模式，0表示16位段（兼容80286），1表示32位段
    
    - `access`:

        bit        | 作用
        ---------- | -
        p          | 段存在位（ Segment Present）。 P 位用于指示描述符所对应的段是否存在, 1表示存在

        DPL 2bit   | 段特权级，0表示特权级0，1表示特权级1，2表示特权级2，3表示特权级3。每当操作系统加载一个用户程序时，它通常都会指定一个稍低的特权级，比如 3 特权级。不同特权级别的程序是互相隔离的，其互访是严格限制的，而且有些处理器指令（特权指令）只能由 0 特权级的程序来执行，为的就是安全。这里再次点明了为何叫保护模式。

        S          | 描述符的类型（ Descriptor Type）。当该位是“ 0”时，表示是一个系统段；为“ 1”时，表示是一个代码段或者数据段（堆栈段也是特殊的数据段）。

        TYPE 4bit  | 指示描述符的子类型，或者说是类别

        - TYPE

            X | E/C | W/R | A | 描述符类别 | 含义
            - | -   | -   | - | --------- | -
            0 | 0   | 0   | X | 数据段     | 只读
            0 | 0   | 1   | X | 数据段     | 可读写
            0 | 1   | 0   | X | 堆栈段     | 只读，向下扩展
            0 | 1   | 1   | X | 堆栈段     | 可读写，向下扩展
            1 | 0   | 1   | X | 代码段     | 可执行、只读
            1 | 1   | 0   | X | 代码段     | 可执行、依从的代码段
            1 | 1   | 1   | X | 代码段     | 可执行、只读、依从的代码段

            > X 表示是否可以执行（ eXecutable）。数据段总是不可执行的， X＝0；代码段总是可以执行的，因此， X＝1。
            > 对于数据段来说， E 位指示段的扩展方向。 E＝0 是向上扩展的，也就是向高地址方向扩展的，是普通的数据段； E＝1 是向下扩展的，也就是向低地址方向扩展的，通常是堆栈段。
            > W 位指示段的读写属性，或者说段是否可写， W＝0 的段是不允许写入的，否则会引发处理器异常中断； W＝1的段是可以正常写入的。

            > 对于代码段来说, C 位指示段是否为特权级依从的（ Conforming）。 C＝0 表示非依从的代码段，这样的代码段可以从与它特权级相同的代码段调用，或者通过门调用； C＝1 表示允许从低特权级的程序转移到该段执行。
            > R 位指示代码段是否允许读出。代码段总是可以执行的，但是，为了防止程序被破坏，它是不能写入的。至于是否有读出的可能，由 R 位指定。 R＝0 表示不能读出，如果企图去读一个 R＝0 的代码段，会引发处理器异常中断；如果 R＝1，则代码段是可以读出的，即可以把这个段的内容当成 ROM 一样使用。

        - GDT例子:
            ```txt
            00000000: (0x00): 未使用的记录表
            10010010: (0x92): 系统专用，可读写的段, 不可执行
            10011010: (0x9a): 系统专用，可执行的段, 可读不可写
            11110010: (0xf2): 应用程序专用，可读写的段, 不可执行
            11111010: (0xfa): 应用程序专用，可执行的段, 可读不可写
            ```


2. 全局段号记录表（GDT）
    
     - 将每个段的信息汇总在一起就是GDT。

3. GDTR指令

    - GDTR寄存器共48位：高32位表示GDT的基址，低16位表示GDT的大小。
    - 使用`LGDT`指令将内存中的GDT信息（地址、长度）拷贝到CPU中。

### IDT

中断记录表（interrupt descriptor table）

#### 初始化PIC

##### PIC中的寄存器

* IMR: 中断屏蔽寄存器，用于屏蔽中断
* IRR: 中断请求寄存器，用于请求中断
* ISR: 中断服务寄存器，用于中断服务

##### PIC

1. 初始化PIC

每个外设（主和从）都有一个命令端口和数据端口（在表中给出）。当没有命令被发出时，数据端口允许我们访问8259 PIC的中断屏蔽。

Chip - Purpose       | I/O port
-------------------- | ---------
Master PIC - Command | 0x0020
Master PIC - Data    | 0x0021
Slave PIC - Command  | 0x00A0
Slave PIC - Data     | 0x00A1

```asm
// IMR：中断屏蔽寄存器
// 屏蔽PIC0和PIC1的所有中断
port_byte_out(PIC0_IMR, 0xff);
port_byte_out(PIC1_IMR, 0xff);

//PIC1使用端口0x20接收命令，0x21接收数据。PIC2使用端口0xA0接收指令，0xA1接收数据。
port_byte_out(PIC0_ICW1, 0x11);    // 设置为边沿触发模式 
port_byte_out(PIC0_ICW2, 0x20);    // IRQ0-7由INT20-27接收
port_byte_out(PIC0_ICW3, 1 << 2);  // PIC1由IRQ2接收
port_byte_out(PIC0_ICW4, 0x01);    // 无缓冲区模式

port_byte_out(PIC1_ICW1, 0x11);    // 设置为边沿触发模式
port_byte_out(PIC1_ICW2, 0x28);    // IRQ8-15由INT28-2f接收
port_byte_out(PIC1_ICW3, 2);       // PIC1由IRQ2接收
port_byte_out(PIC1_ICW4, 0x01);    // 无缓冲区模式

port_byte_out(PIC0_IMR, 0xfb);     // PIC1以外的所有中断都屏蔽
port_byte_out(PIC1_IMR, 0xff);     // 全部屏蔽
```
2. 向PIC发送中断结束信号

发送 EOI 稍微复杂一些：要发送 EOI，你需要把特殊的 EOI 字节（0x60）和相应的 IRQ 号进行 OR。但是，如果你想发送 EOI 到从片，你还需要向主片发送 EOI，在级联引脚上。否则，主片将永远不知道从片中断已经被服务。

例子：

```
// 发送键盘中断EOI给PIC0
// 0x60 + IRQ1
port_byte_out(PIC0_OCW2, 0x61);
```

#### 鼠标中断




#### 键盘中断

1. 从port 0x60获取设备输入的8位按键编码信息。

```asm
in 0x60, al ; al = keycode
```

> 右ctrl按下/松开的编码是两个键码：0xe0 0x1d/0x9d, 会产生两次中断

2. 判断键盘控制电路是否可以处理CPU指令

```asm
in 0x64, al ; al = status
; 如果al的bit1为0，则可以处理指令
```

### pushfl popfl

- pushfl 将寄存器的标志位压入栈中
    等同于：
    ```S
    push eax
    push ecx
    push edx
    push ebx
    push esp
    push ebp
    push esi
    push edi
    ```
- popfl 将栈顶的标志位弹出到寄存器中

**不能用mov 操作标志寄存器**
