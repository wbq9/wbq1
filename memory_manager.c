#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 最大ID字符长度
#define MAX_ID_LEN 20

// 已分配内存块结构体
typedef struct AllocBlock {
	char id[MAX_ID_LEN];
	long long start;
	long long size;
	struct AllocBlock *next;
} AllocBlock;

// 空闲内存块结构体
typedef struct FreeBlock {
	long long start;
	long long size;
	struct FreeBlock *next;
} FreeBlock;

// 全局链表头
AllocBlock *alloc_head = NULL;
FreeBlock *free_head = NULL;
// 总内存大小
long long total_mem;

// ===================== 函数声明 =====================
// 初始化内存：整块内存初始为空闲块
void init_memory(long long M);

// 查找已分配块，返回指针；找不到返回NULL
AllocBlock* find_alloc_block(const char *id);

// BEST FIT最佳适应算法：选最小且能放下size的空闲块，同尺寸选地址最小
FreeBlock* best_fit(long long size);

// 分配内存 ALLOC id size
void alloc_memory(const char *id, long long size);

// 释放内存 FREE id
void free_memory(const char *id);

// 合并相邻空闲块（释放后调用）
void merge_free_blocks();

// 查询内存块 QUERY id
void query_block(const char *id);

// 内存紧凑整理 COMPACT
void compact_memory();

// 打印内存状态 DUMP
void dump_memory();

// 释放所有链表内存，程序退出时调用
void free_all_list();
// ===================== 函数实现 =====================// ===================== Day2 函数实现 =====================
void init_memory(long long M) {
	total_mem = M;
	free_head = (FreeBlock*)malloc(sizeof(FreeBlock));
	free_head->start = 0;
	free_head->size = M;
	free_head->next = NULL;
	alloc_head = NULL;
}

AllocBlock* find_alloc_block(const char *id) {
	AllocBlock *p = alloc_head;
	while (p != NULL) {
		if (strcmp(p->id, id) == 0) {
			return p;
		}
		p = p->next;
	}
	return NULL;
}

FreeBlock* best_fit(long long size) {
	FreeBlock *p = free_head;
	FreeBlock *target = NULL;
	while (p != NULL) {
		if (p->size >= size) {
			if (target == NULL) {
				target = p;
			} else {
				// 更小的块 或者 同尺寸地址更小
				if (p->size < target->size || (p->size == target->size && p->start < target->start)) {
					target = p;
				}
			}
		}
		p = p->next;
	}
	return target;
}

void alloc_memory(const char *id, long long size) {
	// 1. ID重复校验
	if (find_alloc_block(id) != NULL) {
		printf("ALLOC_FAILED %s DUPLICATE\n", id);
		return;
	}
	// 2. 最佳适应找空闲块
	FreeBlock *fit = best_fit(size);
	if (fit == NULL) {
		printf("ALLOC_FAILED %s NO_SPACE\n", id);
		return;
	}
	long long alloc_start = fit->start;
	// 3. 创建已分配块节点，头插法插入分配链表
	AllocBlock *new_alloc = (AllocBlock*)malloc(sizeof(AllocBlock));
	strcpy(new_alloc->id, id);
	new_alloc->start = alloc_start;
	new_alloc->size = size;
	new_alloc->next = alloc_head;
	alloc_head = new_alloc;
	
	// 4. 更新空闲块：分配剩余部分
	if (fit->size == size) {
		// 整块分配，删除空闲节点
		FreeBlock *pre = NULL, *cur = free_head;
		while (cur != fit) {
			pre = cur;
			cur = cur->next;
		}
		if (pre == NULL) {
			free_head = cur->next;
		} else {
			pre->next = cur->next;
		}
		free(cur);
	} else {
		// 剩余空闲空间，修改起始地址和大小
		fit->start += size;
		fit->size -= size;
	}
	printf("ALLOCATED %s %lld\n", id, alloc_start);
}


