# C语言 动态内存管理器（暑期考核项目）
基于链表实现最佳适应(Best Fit)内存分配、空闲块合并、内存紧凑整理、状态打印功能。

## 功能列表
1. `ALLOC id size`：最佳适应分配内存
2. `FREE id`：释放内存+相邻空闲块合并
3. `QUERY id`：查询指定分配块信息
4. `COMPACT`：内存紧凑整理，块前移消除碎片
5. `DUMP`：打印全部已分配、空闲内存块状态

## 编译运行
### 编译（GCC编译器）
```bash
gcc memory_manager.c -o mem_manager
