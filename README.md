# 基于链表的内存管理

## 配置功能
1. 内存池数量和大小

    在Malloc.h中可以根据资源自由配置内存池数量和内存池大小，分别由如下宏实现：
   1. 内存池数量:`MEMORY_POLL_NUM`
   2. 内存池大小:`MEM0_MAX_SIZE`, `MEM1_MAX_SIZE`, `MEM2_MAX_SIZE`,
2. 最大化利用碎片内存功能

    在Malloc.h中可以根据资源自由配置是否最大化利用碎片内存，由如下两个宏实现：
    1. 是否最大化利用碎片内存:`MEMORY_FRAGMENT_OPT` 开启此宏的定义即可开启最大化利用碎片内存功能
    2. 搜索合理内存深度:`FIND_MAX_DEEP` 这个宏的大小决定了搜索最优内存的链表深度，越大搜索深度，越耗时

## API介绍
### 初始化
`void mem_init();` 在调用本库API之前调用此函数，初始化内存管理
### 内存申请与释放
1. `void *mymalloc(MemPoolNum_e mem_num, uint32_t size);`
    ```c
    /**
     * @brief 内存申请
     * @note  4字节对齐
     * @param mem_num 内存池编号 @ref MemPoolNum_e
     * @param size 申请内存大小(单位:字节)
     * @return 申请到的内存首地址
     */
     void *mymalloc(uint8_t mem_num, uint32_t size)
    ```

2. `void *myrealloc(MemPoolNum_e mem_num, void *ptr, uint32_t size);`
    ```c
   /**
    * @brief 重新申请内存
    * @param mem_num 内存池编号 @ref MemPoolNum_e
    * @param ptr 内存首地址
    * @param size 申请内存大小(单位:字节)
    * @return 申请到的内存首地址
    */
     void *myrealloc(uint8_t mem_num, void *ptr, uint32_t size)
    ```
3. `void myfree(void *ptr);`
    ```c
    /**
    * @brief 释放内存
    * @param ptr 内存首地址
    */
    void myfree(void *ptr)
    ```

## 移植到单片机中
本工程是在PC上编写运行并测试的，但是也移植到单片机中进行了测试，没有任何问题，但是这里需要根据单片机芯片手册做出调整

在单片机中进行内存管理需要查询芯片手册中的SRAM地址来指定内存池的地址，不同的编译器语法不同，这里给出Keil示例
```c
#if MEMORY_POLL_NUM == 1
__align(4) uint8_t membase0[MEM0_MAX_SIZE];    // 内部SRAM内存池
MemeyNode HedaNode[1], EndNode[1];              // 头尾节点
uint32_t mem_size_table[1] = {MEM0_MAX_SIZE};   // 内存size表
uint8_t *membase_table[1] = {membase0};         // 内存基地址表
#elif MEMORY_POLL_NUM == 2
__align(4) uint8_t membase0[MEM0_MAX_SIZE];                                // 内部SRAM内存池
__align(4) uint8_t membase1[MEM1_MAX_SIZE] __attribute__((at(0X10000000)));// CCM内存池
MemeyNode HedaNode[2], EndNode[2];                           // 头尾节点
uint32_t mem_size_table[2] = {MEM0_MAX_SIZE, MEM1_MAX_SIZE}; // 内存size表
uint8_t *membase_table[2] = {membase0, membase1};            // 内存基地址表
#elif MEMORY_POLL_NUM == 3
__align(4) uint8_t membase0[MEM0_MAX_SIZE];                                    // 内部SRAM内存池
__align(4) uint8_t membase1[MEM1_MAX_SIZE] __attribute__((at(0X10000000)));    // CCM内存池
__align(4) uint8_t membase2[MEM2_MAX_SIZE] __attribute__((at(0X20020000)));    // SRAM2内存池
MemeyNode HedaNode[3], EndNode[3];                                              // 头尾节点
uint32_t mem_size_table[3] = {MEM0_MAX_SIZE, MEM1_MAX_SIZE, MEM2_MAX_SIZE};     // 内存size表
uint8_t *membase_table[3] = {membase0, membase1, membase2};                     // 内存基地址表
#elif MEMORY_POLL_NUM == 4
__align(4) uint8_t membase0[MEM0_MAX_SIZE];                                          // 内部SRAM内存池
__align(4) uint8_t membase1[MEM1_MAX_SIZE] __attribute__((at(0X10000000)));          // CCM内存池
__align(4) uint8_t membase2[MEM2_MAX_SIZE] __attribute__((at(0X20020000)));          // SRAM2内存池
__align(4) uint8_t membase3[MEM3_MAX_SIZE] __attribute__((at(0X20030000)));          // SRAM3内存池
MemeyNode HedaNode[4], EndNode[4];                                                    // 头尾节点
uint32_t mem_size_table[4] = {MEM0_MAX_SIZE, MEM1_MAX_SIZE, MEM2_MAX_SIZE, MEM3_MAX_SIZE};   // 内存size表
uint8_t *membase_table[4] = {membase0, membase1, membase2, membase3};                 // 内存基地址表
#endif

```
