/**
 * @file Malloc.c
 * @author GYM (480609450@qq.com)
 * @brief 
 * @version 0.1
 * @date 2025-09-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "Malloc.h"
#include "stdio.h"

typedef struct memy_node *PNode; // 定义节点指针
typedef struct memy_node
{
    uint32_t offset; // 与内存首地址的偏移
    uint32_t size;   // 本节点带的内存大小
    PNode prior;     // 前驱指针域
    PNode next;      // 后继指针域
    uint32_t mark;   // 标记，避免释放错误数据
} MemoryNode;

#define MEMORY_MARK (uint32_t)0x1F57F9D2

/* 编译器识别 */
#if defined(__IAR_SYSTEMS_ICC__) || defined(__ICCARM__)
    /* 当前是 IAR 编译器 */
    #define COMPILER_IAR
#elif defined(__CC_ARM)
    /* 当前是 Keil Arm Compiler 5 (AC5) */
    #define COMPILER_KEIL_AC5
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6000000)
    /* 当前是 Keil Arm Compiler 6 (AC6) */
    #define COMPILER_KEIL_AC6
#else
    /* 未知编译器 */
    #define COMPILER_UNKNOW
#endif


/* 通过不同编译器的语法来指定内存池地址 */
#if defined(COMPILER_KEIL_AC5)
			#define MEMORY_POOL_INOF(serial, size, name, address) __align(4) uint8_t mem_##name[size]__attribute__((at(address)));
					MEMORY_POOL_TABLE
			#undef MEMORY_POOL_INOF
#elif defined(COMPILER_KEIL_AC6)
			    #define MEMORY_POOL_INOF(serial, size, name, address) \
        __attribute__((aligned(4))) \
        __attribute__((section(".ARM.__at_" #address))) \
        uint8_t mem_##name[size];
					MEMORY_POOL_TABLE
			#undef MEMORY_POOL_INOF
#elif defined(COMPILER_IAR)
#define MEMORY_POOL_INOF(serial, size, name, address) __no_init uint8_t mem_##name[size] @ address;
           MEMORY_POOL_TABLE
#undef MEMORY_POOL_INOF
#elif defined(COMPILER_UNKNOW)
#define MEMORY_POOL_INOF(serial, size, name, address) uint8_t mem_##name[size];
           MEMORY_POOL_TABLE
#undef MEMORY_POOL_INOF
#endif

// 内存池枚举
static const MemPool_e memory_enum_table[] = {
    #define MEMORY_POOL_INOF(serial, size, name, address) name,
    MEMORY_POOL_TABLE
    #undef MEMORY_POOL_INOF
};

#define MEMORY_POOL_MAX (sizeof(memory_enum_table) / sizeof(memory_enum_table[0]))

// 头 尾节点
static MemoryNode HeadNode[MEMORY_POOL_MAX], EndNode[MEMORY_POOL_MAX]; 

// 内存大小表
static uint32_t memory_size_table[MEMORY_POOL_MAX] = {
#define MEMORY_POOL_INOF(serial, size, name, address) size,
           MEMORY_POOL_TABLE
#undef MEMORY_POOL_INOF
}; 

// 内存基地址表
static uint8_t *memory_base_table[MEMORY_POOL_MAX] = {
#define MEMORY_POOL_INOF(serial, size, name, address) mem_##name,
           MEMORY_POOL_TABLE
#undef MEMORY_POOL_INOF
}; 

// 内存池名字
static const char *memory_name_table[] = {
    #define MEMORY_POOL_INOF(serial, size, name, address) #name,
           MEMORY_POOL_TABLE
#undef MEMORY_POOL_INOF
};


/**
 * @brief 格式化内存
 * @param dest 内存首地址
 * @param value 格式化值
 * @param size 长度
 */
void lw_memset(void *dest, uint8_t value, uint32_t size)
{
    uint8_t *addr = dest;
    while (size--)
    {
        *addr++ = value;
    }
}

/**
 * @brief 内存复制
 * @param dest 目标地址
 * @param src 源地址
 * @param size 长度
 */
void lw_memcpy(void *dest, void *src, uint32_t size)
{
    uint8_t *addr = dest;
    uint8_t *addr1 = src;
    while (size--)
    {
        *addr++ = *addr1++;
    }
}

/**
 * @brief 初始化内存
 */
void lw_malloc_init() // 初始化内存链表
{
    for (int i = 0; i < MEMORY_POOL_MAX; ++i)
    {
        HeadNode[i].offset = 0;
        HeadNode[i].size = 0;
        HeadNode[i].prior = NULL;
        HeadNode[i].next = &EndNode[i];
        HeadNode[i].mark = MEMORY_MARK;

        EndNode[i].offset = memory_size_table[i];
        EndNode[i].size = 0;
        EndNode[i].prior = &HeadNode[i];
        EndNode[i].next = NULL;

        lw_memset(memory_base_table[i], 0, memory_size_table[i]);
    }
}


#ifndef MEMORY_FRAGMENT_OPT // 内存碎片优化
/**
 * @brief 内存申请
 * @note  4字节对齐
 * @param mem_num 内存池编号 @ref MemPool_e
 * @param size 申请内存大小(单位:字节)
 * @return 申请到的内存首地址
 */
void *lw_malloc(MemPool_e mem_num, uint32_t size)
{
    if (mem_num >= MEMORY_POOL_MAX)
    {
        printf("内存池编号错误\n");
        return NULL;
    }
    PNode p = NULL;
    PNode pnew = NULL;
    uint32_t relsize;
    uint8_t *menaddr;
    relsize = size + sizeof(MemoryNode); // 实际要申请的内存大小 包含节点空间
    if (relsize % 4)                    // 4字节对齐
        relsize += (4 - (relsize % 4)); // 不满4字节就补齐
    p = &HeadNode[mem_num];                      // 导入头节点
    while (p->next)                     // 下一节点存在
    {
        if ((p->next->offset - (p->offset + p->size)) > relsize) // 本节点和下节点间剩余的内存>要申请的内存大小
        {
            menaddr = memory_base_table[mem_num] + (p->offset) + (p->size);
            pnew = (PNode)menaddr; // 新节点的地址=内存池基地址+本节点偏移量+本节点内存大小
            pnew->prior = p;
            pnew->next = p->next;                  // 新节点的Pnext=本节点Pnext
            pnew->size = relsize;                  // 新节点的内存大小
            pnew->offset = p->offset + p->size;    // 新节点偏移量
            pnew->mark = MEMORY_MARK;
            p->next = pnew;                        // 本节点Pnext=新节点的地址
            lw_memset(pnew + 1, 0, size);           // 初始化内存
            return (void *)(((PNode)menaddr) + 1); // 返回去除节点信息的地址
        }
        else
            p = p->next; // 导入下一个节点
    }
    return NULL; // 申请失败
}
#else
/**
 * @brief 内存申请
 * @note  4字节对齐 最大化利用碎片内存
 * @param mem_num 内存池编号 @ref MemPool_e
 * @param size 申请内存大小(单位:字节)
 * @return 申请到的内存首地址
 */
void *lw_malloc(MemPool_e mem_num, uint32_t size)
{
    if (mem_num >= MEMORY_POOL_MAX)
    {
        printf("内存池编号错误\n");
        return NULL;
    }
    PNode p = NULL;
    PNode pnew = NULL;
    uint32_t relsize;
    uint8_t *menaddr;
    relsize = size + sizeof(MemoryNode); // 实际要申请的内存大小 包含节点空间
    if (relsize % 4)                    // 4字节对齐 不满4字节就补齐
    {
        relsize += (4 - (relsize % 4));
    }
    p = &HeadNode[mem_num];             // 导入头节点
    uint8_t node_deep = 0;              // 节点深度
    uint8_t valid_noed_num = 0;         // 有效节点数量
    uint32_t min_szie = 0xFFFFFFFF;     // 最小有效节点大小
    uint8_t min_size_deep = 0;          // 最小有效节点深度
    while (p->next)                     //寻找最佳大小的节点
    {
        uint32_t free_size = p->next->offset - (p->offset + p->size);
        if (free_size >= relsize) // 本节点和下节点间剩余的内存>=要申请的内存大小
        {
            if (free_size < min_szie)
            {
                min_szie = free_size;
                min_size_deep = node_deep;
            }
            if (++valid_noed_num >= FIND_MAX_DEEP)
                break;
        }
        p = p->next;
        node_deep++;
    }
    if (valid_noed_num == 0)    // 没有找到有效节点
    {
        return NULL;
    }

    p = &HeadNode[mem_num];             // 重新导入头节点
    for (int i = 0; i < min_size_deep; ++i)
    {
        p = p->next;
    }
    menaddr = memory_base_table[mem_num] + (p->offset) + (p->size);
    pnew = (PNode)menaddr; // 新节点的地址=内存池基地址+本节点偏移量+本节点内存大小
    pnew->prior = p;
    pnew->next = p->next;                  // 新节点的Pnext=本节点Pnext
    pnew->size = relsize;                  // 新节点的内存大小
    pnew->offset = p->offset + p->size;    // 新节点偏移量
    pnew->mark = MEMORY_MARK;
    p->next = pnew;                        // 本节点Pnext=新节点的地址
    lw_memset(pnew + 1, 0, size);           // 初始化内存
    return (void *)(((PNode)menaddr) + 1); // 返回去除节点信息的地址
}
#endif

/**
 * @brief 自动选择内存池分配内存
 * @param size 申请内存大小(单位:字节)
 * @return 申请到的内存首地址
 */
void *lw_malloc_auto(uint32_t size)
{
    uint8_t *p = NULL;
    for (uint16_t i = 0; i < MEMORY_POOL_MAX; i++)
    {
        p = (uint8_t *)lw_malloc(memory_enum_table[i], size);
        if (p != NULL)
        {
            break;
        }
    }
    return (void *)p;
}

/**
 * @brief 重新申请内存
 * @param mem_num 内存池编号 @ref MemPool_e
 * @param ptr 内存首地址
 * @param size 申请内存大小(单位:字节)
 * @return 申请到的内存首地址
 */
void *lw_realloc(MemPool_e mem_num, void *ptr, uint32_t size)
{
    if (ptr == NULL)
        return lw_malloc(mem_num, size);
    if (size == 0)
    {
        lw_free(ptr);
        return NULL;
    }
    PNode p = ((PNode)ptr) - 1;
    if (p->size >= size)
    {
        return ptr;
    }
    uint32_t relsize = size + sizeof(MemoryNode);
    uint32_t behind_size = p->next->offset - (p->offset + p->size); // 本节点和下节点间剩余的 优先寻找原地址下是否有充足空间
    if (relsize % 4)
    {
        relsize += (4 - (relsize % 4));
    }
    if (behind_size + p->size >= relsize)
    {
        p->size = relsize;
        return ptr;
    }

    uint8_t *menaddr = (uint8_t *)lw_malloc(mem_num, size);  // 原地址下空间不足， 申请新的内存
    if (menaddr == NULL)
    {
        printf("malloc error\n");
        return NULL;
    }
    lw_memcpy(menaddr, ptr, p->size);    // 内存拷贝
    lw_free(ptr);
    return (void *)menaddr;
}


/**
 * @brief 释放内存
 * @param ptr 内存首地址
 */
void lw_free(void *ptr)
{
    PNode p = NULL;
    if (ptr == NULL)
        return;
    p = ((PNode)ptr) - 1;      // 点位到内存地址前的节点信息
    if (p->prior && p->next && p->mark == MEMORY_MARK) // 防止二次释放产生错误或释放到非内存池内存
    {
        p->prior->next = p->next;  // 本上节点的上一节点的Pnext=本节点的下一节点地址
        p->next->prior = p->prior; // 本上节点的下一节点的Prior=本节点的上一节点地址
        p->prior = NULL;
        p->next = NULL;
    }
}

/**
 * @brief 获取内存池使用率
 * @param mem_num 内存池编号 @ref MemPool_e
 */
float lw_get_memory_rate(MemPool_e mem_num)
{
    uint32_t use_size = 0;
    float use_rate = 0.0f;
    PNode p = &HeadNode[mem_num];
    while (p->next)
    {
        use_size = (p->size == 0) ? use_size : (use_size + p->size + sizeof(MemoryNode));
        p = p->next;
    }
    use_rate = (use_size == 0) ? 0.0f: ((float)use_size * 100.0f / (float)memory_size_table[mem_num]);
    return use_rate;
}

/**
 * @brief 获取内存池详细信息
 * @note 返回格式化后的字符串
 * @param[out] buffer 缓冲区
 */
void lw_memory_list(uint8_t *buffer, uint16_t buffer_size)
{
    uint32_t use_size = 0;
    float use_rate = 0.0f;
    PNode p = NULL;
    uint16_t output_size = 0;
    output_size += snprintf(buffer, buffer_size, "poll          total(KB)  used(KB)  rate(%%)\r\n*****************************************\r\n");
    for (uint16_t i = 0; i < MEMORY_POOL_MAX; i++)
    {
        p = &HeadNode[i];
        use_size = 0;
        while (p->next)
        {
            use_size = (p->size == 0) ? use_size : (use_size + p->size + sizeof(MemoryNode));
            p = p->next;
        }
        use_rate = (use_size == 0) ? 0.0f: ((float)use_size * 100.0f / (float)memory_size_table[i]);
        output_size += snprintf(buffer + output_size, buffer_size - output_size, "%-15s %-4d      %-4d      %.2f\r\n", memory_name_table[i], (memory_size_table[i] / 1024), (use_size / 1024), use_rate);
    }
}
