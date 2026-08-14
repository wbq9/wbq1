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

void merge_free_blocks() {
	if (free_head == NULL || free_head->next == NULL) return;
	// 第一步：按起始地址升序排序空闲链表
	FreeBlock *p = free_head;
	FreeBlock sorted_head = {0,0,NULL};
	while (p != NULL) {
		FreeBlock *cur = p;
		p = p->next;
		FreeBlock *insert = &sorted_head;
		while (insert->next != NULL && insert->next->start < cur->start) {
			insert = insert->next;
		}
		cur->next = insert->next;
		insert->next = cur;
	}
	free_head = sorted_head.next;
	
	// 第二步：遍历合并相邻块
	p = free_head;
	while (p != NULL && p->next != NULL) {
		long long end = p->start + p->size;
		if (end == p->next->start) {
			// 相邻，合并
			FreeBlock *temp = p->next;
			p->size += temp->size;
			p->next = temp->next;
			free(temp);
		} else {
			p = p->next;
		}
	}
}

void free_memory(const char *id) {
	AllocBlock *del = find_alloc_block(id);
	if (del == NULL) {
		printf("FREE_FAILED %s NOT_FOUND\n", id);
		return;
	}
	long long s = del->start;
	long long sz = del->size;
	
	// 从分配链表删除节点
	AllocBlock *pre = NULL, *cur = alloc_head;
	while (cur != del) {
		pre = cur;
		cur = cur->next;
	}
	if (pre == NULL) {
		alloc_head = cur->next;
	} else {
		pre->next = cur->next;
	}
	free(cur);
	
	// 新建空闲块，插入链表，然后合并
	FreeBlock *new_free = (FreeBlock*)malloc(sizeof(FreeBlock));
	new_free->start = s;
	new_free->size = sz;
	new_free->next = free_head;
	free_head = new_free;
	merge_free_blocks();
	
	printf("FREED %s %lld %lld\n", id, s, sz);
}

void query_block(const char *id) {
	AllocBlock *blk = find_alloc_block(id);
	if (blk == NULL) {
		printf("BLOCK_NOT_FOUND %s\n", id);
	} else {
		printf("BLOCK %s %lld %lld\n", blk->id, blk->start, blk->size);
	}
}

// 辅助：对分配块按start升序排序
static void sort_alloc_by_start(AllocBlock **head) {
	if (*head == NULL || (*head)->next == NULL) return;
	AllocBlock dummy = {"",0,0,NULL};
	AllocBlock *p = *head;
	while (p != NULL) {
		AllocBlock *cur = p;
		p = p->next;
		AllocBlock *ins = &dummy;
		while (ins->next != NULL && ins->next->start < cur->start) {
			ins = ins->next;
		}
		cur->next = ins->next;
		ins->next = cur;
	}
	*head = dummy.next;
}

void compact_memory() {
	sort_alloc_by_start(&alloc_head);
	AllocBlock *p = alloc_head;
	long long cur_addr = 0;
	int move_cnt = 0;
	// 临时链表保存移动记录
	typedef struct MoveLog {
		char id[MAX_ID_LEN];
		long long old_s;
		long long new_s;
		struct MoveLog *next;
	} MoveLog;
	MoveLog log_head = {"",0,0,NULL};
	MoveLog *log_tail = &log_head;
	
	while (p != NULL) {
		if (p->start != cur_addr) {
			// 发生移动，记录日志
			MoveLog *log = (MoveLog*)malloc(sizeof(MoveLog));
			strcpy(log->id, p->id);
			log->old_s = p->start;
			log->new_s = cur_addr;
			log->next = NULL;
			log_tail->next = log;
			log_tail = log;
			move_cnt++;
			p->start = cur_addr;
		}
		cur_addr += p->size;
		p = p->next;
	}
	
	// 清空原有空闲链表，新建末尾唯一空闲块
	FreeBlock *fp = free_head;
	while (fp != NULL) {
		FreeBlock *tmp = fp;
		fp = fp->next;
		free(tmp);
	}
	free_head = NULL;
	if (cur_addr < total_mem) {
		free_head = (FreeBlock*)malloc(sizeof(FreeBlock));
		free_head->start = cur_addr;
		free_head->size = total_mem - cur_addr;
		free_head->next = NULL;
	}
	
	// 输出结果
	printf("COMPACTED %d\n", move_cnt);
	MoveLog *log_p = log_head.next;
	while (log_p != NULL) {
		printf("%s %lld %lld\n", log_p->id, log_p->old_s, log_p->new_s);
		MoveLog *tmp = log_p;
		log_p = log_p->next;
		free(tmp);
	}
}

void dump_memory() {
	// 拷贝并排序分配块
	AllocBlock tmp_head = {"",0,0,NULL};
	AllocBlock *p = alloc_head;
	while (p != NULL) {
		AllocBlock *new_n = (AllocBlock*)malloc(sizeof(AllocBlock));
		strcpy(new_n->id, p->id);
		new_n->start = p->start;
		new_n->size = p->size;
		new_n->next = tmp_head.next;
		tmp_head.next = new_n;
		p = p->next;
	}
	sort_alloc_by_start(&tmp_head.next);
	
	// 打印已分配块
	int alloc_cnt = 0;
	AllocBlock *ap = tmp_head.next;
	while (ap != NULL) {
		alloc_cnt++;
		ap = ap->next;
	}
	printf("ALLOCATED_BLOCKS %d\n", alloc_cnt);
	ap = tmp_head.next;
	while (ap != NULL) {
		printf("%s %lld %lld\n", ap->id, ap->start, ap->size);
		AllocBlock *tmp = ap;
		ap = ap->next;
		free(tmp);
	}
	
	// 排序空闲块并打印
	merge_free_blocks();
	int free_cnt = 0;
	FreeBlock *fp = free_head;
	while (fp != NULL) {
		free_cnt++;
		fp = fp->next;
	}
	printf("FREE_BLOCKS %d\n", free_cnt);
	fp = free_head;
	while (fp != NULL) {
		printf("%lld %lld\n", fp->start, fp->size);
		fp = fp->next;
	}
}
