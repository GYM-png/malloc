#include <stdio.h>
#include <stdint.h>
#include "Malloc.h"


/**
 * @brief 检查内存数据
 * @param addr 地址
 * @param value 目标值
 * @param size 长度
 */
void mem_ceck(const uint8_t *addr, const uint8_t value, uint32_t size)
{
    if (addr == NULL)
    {
        printf("addr is null\n");
        return;
    }
    for (int i = 0; i < size; ++i) {
        if (addr[i] != value)
        {
            printf("%p check error at %d\n", addr, i);
            return;
        }
    }
    printf("check success\n");
}

#define DATA1_VALUE 0xAA
#define DATA2_VALUE 0xBB
#define DATA3_VALUE 0xCC
#define DATA4_VALUE 0xDD
#define DATA5_VALUE 0xEE
#define DATA6_VALUE 0xFF

#define DATA1_SIZE (1024 * 1)
#define DATA2_SIZE (1024 * 3)
#define DATA3_SIZE (1024 * 5)
#define DATA4_SIZE (1024 * 2)
#define DATA5_SIZE (1024 * 1)
#define DATA6_SIZE (1024 * 2)
int main(void) {
    mem_init();
    uint8_t *data1 = NULL, *data2 = NULL, *data3 = NULL, *data4 = NULL, *data5 = NULL, *data6 = NULL;
    data1 = (uint8_t *)mymalloc(MEM_NUM0, DATA1_SIZE);
    data2 = (uint8_t *)mymalloc(MEM_NUM0, DATA2_SIZE);
    data3 = (uint8_t *)mymalloc(MEM_NUM0, DATA3_SIZE);
    data4 = (uint8_t *)mymalloc(MEM_NUM0, DATA4_SIZE);
    data5 = (uint8_t *)mymalloc(MEM_NUM0, DATA5_SIZE);
    if(data1 == NULL || data2 == NULL || data3 == NULL || data4 == NULL || data5 == NULL)
    {
        printf("malloc error\n");
        if (data1)
            myfree(data1);
        if (data2)
            myfree(data2);
        if (data3)
            myfree(data3);
        if (data4)
            myfree(data4);
        if (data5)
            myfree(data5);
        return -1;
    }
    mymemset(data1, DATA1_VALUE, DATA1_SIZE);
    mymemset(data2, DATA2_VALUE, DATA2_SIZE);
    mymemset(data3, DATA3_VALUE, DATA3_SIZE);
    mymemset(data4, DATA4_VALUE, DATA4_SIZE);
    mymemset(data5, DATA5_VALUE, DATA5_SIZE);

    mem_ceck(data1, DATA1_VALUE, DATA1_SIZE);
    mem_ceck(data2, DATA2_VALUE, DATA2_SIZE);
    mem_ceck(data3, DATA3_VALUE, DATA3_SIZE);
    mem_ceck(data4, DATA4_VALUE, DATA4_SIZE);
    mem_ceck(data5, DATA5_VALUE, DATA5_SIZE);
    // 此时的内存分配情况
    // data1 | data2 | data3 | data4 | data5 |
    //| 1K   | 3K    | 5K    | 2K    | 1K    |
    myfree(data2); data2 = NULL;
    myfree(data4); data4 = NULL;
    // 此时的内存分配情况
    // data1 |       | data3 |       | data5 |
    //| 1K   | 3K    | 5K    | 2K    | 1K    |

    data6 = (uint8_t *)mymalloc(MEM_NUM0, DATA6_SIZE);//DATA6_SIZE 为2K， 开启内存碎片优化后，会申请到原data4(2K)的位置，否则会申请到原data2(3K)的位置
    if (data6 == NULL)
    {
        printf("内存申请失败\n");
        return -1;
    }
    // 此时的内存分配情况
    // data1 |       | data3 | data6 | data5 |
    //| 1K   | 3K    | 5K    | 2K    | 1K    |

    mymemset(data6, DATA6_VALUE, DATA6_SIZE);
    printf("brefore data 5 addr %p\n", data5);
    data5 = (uint8_t *)myrealloc(MEM_NUM0, data5, 1024 * 2);
    printf("after data 5 addr %p\n", data5);
    if (data5 == NULL)
    {
        printf("内存申请失败\n");
    }
    else
    {
        mymemset(data5, DATA5_VALUE, 1024 * 2);
    }
    // 此时的内存分配情况
    // data1 |       | data3 | data6 | data5 |
    //| 1K   | 3K    | 5K    | 2K    | 2K    |
    myfree(data2);
    printf("brefore data 1 addr %p\n", data1);
    data1 = (uint8_t *)myrealloc(MEM_NUM0, data1, 1024 * 5); //这里申请必须大于1024 * 4 + 24 字节，才能使data1首地址改变
    if (data1 == NULL)
    {
        printf("内存申请失败\n");
    }
    else
    {
        mymemset(data1, DATA1_VALUE, 1024 * 5);
    }
    printf("after data 1 addr %p\n", data1);
    // 此时的内存分配情况
    //       |       | data3 | data6 | data5 | data1 |
    //| 1K   | 3K    | 5K    | 2K    | 2K    | 5K    |
    mem_ceck(data1, DATA1_VALUE, 1024 * 5);
    mem_ceck(data2, DATA2_VALUE, DATA2_SIZE);
    mem_ceck(data3, DATA3_VALUE, DATA3_SIZE);
    mem_ceck(data4, DATA4_VALUE, DATA4_SIZE);
    mem_ceck(data5, DATA5_VALUE, DATA5_SIZE);


    myfree(data1);
    myfree(data2);
    myfree(data3);
    myfree(data4);
    myfree(data5);
    myfree(data6);
    return 0;

}
