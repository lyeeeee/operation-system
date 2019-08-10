# 一个os实现
学习os所用，打算实现（copy大法）一个基本功能俱全的os

- log: 2019-6-9 完成对gdt表的初始化，并进入了保护模式
- log: 2019-6-10 写获取内存，还是有bug
- log: 2019-6-11 改掉了内存检测的bug
- log: 2019-6-12 初始化一级页表二级页表，映射内核至虚拟空间3gb以上
- log: 2019-6-21 将内核加载进指定内存处，并初始化内核至编译虚拟地址处
- log: 2019-7-5 实现了第一个打印函数，但是在加载内核的时候出了bug，不得不说搞os最恶心的就是debug，忙了一段实验室终于又继续了
- log: 2019-7-7 增加打印整数的函数
- log: 2019-8-2 添加中断符号表，和简单的中断处理程序，初始化8259A芯片，并进行中断测试
- log: 2019-8-3 改用c语言编写中断处理程序
- log: 2019-8-3 设置8235计数器，增加IRQ引脚上的时钟中断信号频率
- log: 2019-8-3 增加ASSERT断言
- log: 2019-8-4 增加位图结构
- log: 2019-8-4 初始化内存池
- log: 2019-8-5 以页为单位的内存分配，内核和用户内存分别从各自的内存池中分配
- log: 2019-8-6 增加内核线程数据结构，初步跑起来一个线程
- log: 2019-8-6 增加链表结构，用来连接线程进程队列
- log: 2019-8-6 多内核线程调度
- log: 2019-8-8 增加锁结构，利用锁控制控制台输出，并增加键盘驱动
- log: 2019-8-9 增加键盘输入环形缓冲区
- log: 2019-8-10 定义任务状态段TSS
