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

## ORG 0x7c00

> 指示编译器代码将加载到0x7c00

[org 0x7c00解疑](https://blog.csdn.net/judyge/article/details/52333656)

## $

> 当前指令地址

## JC

> 如果寄存器 CF 为 1, 则跳转到指定地址 jump if carry

```S
JC error
```

## JBE JE

和CMP指令协同使用。

> JBE 小于或等于 jump if below or equal
> JE 等于 jump if equal

## INT

> 异常中断

## pushfl popfl

> pushfl 将寄存器的标志位压入栈中
< popfl 将栈顶的标志位弹出到寄存器中

**不能用mov 操作标志寄存器**