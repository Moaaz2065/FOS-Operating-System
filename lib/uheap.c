//#include <inc/lib.h>
#include <inc/lib.h>
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}


//=================================
// [2] ALLOCATE SPACE IN USER HEAP:

//=================================

LIST_HEAD(marked_list,marked);
struct marked {
	uint32 va;
	int pages;
	LIST_ENTRY(marked) prev_next_info;
};

struct marked_list list;


void* malloc(uint32 size)
{

	//DON'T CHANGE THIS CODE========================================
	//TODO: [PROJECT'23.MS2 - #09] [2] USER HEAP - malloc() [User Side]
	InitializeUHeap();
	if (size == 0) return NULL ;
	if(size<=DYN_ALLOC_MAX_BLOCK_SIZE) {
		return alloc_block_FF(size);
	}

	size = ROUNDUP(size,PAGE_SIZE);
	uint32 s = sys_limit() + PAGE_SIZE;
	int cnt = 0;
	uint32 pva;
	for(uint32 i = s ; i < USER_HEAP_MAX ; i += PAGE_SIZE)
	{
		if(sys_marked(i)) cnt++;
		else cnt = 0;
		if(cnt == 1) pva = i;
		if(cnt == size/PAGE_SIZE) break;
	}
	if(cnt != size/PAGE_SIZE) return NULL;
	struct marked *temp = alloc_block_FF(sizeof(struct marked));
	temp->va = pva;
	temp->pages = size/PAGE_SIZE;
	LIST_INSERT_TAIL(&list,temp);
	sys_allocate_user_mem(pva,size);
	return (void *)pva;
}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #11] [2] USER HEAP - free() [User Side]
	// Write your code here, remove the panic and write your code
	uint32 vva = (uint32) virtual_address;
	if(vva < (uint32)sbrk(0) && vva >= USER_HEAP_START) {
		free_block(virtual_address);
		return;
	}
	else if(vva >= (uint32)sbrk(0)+PAGE_SIZE && vva < USER_HEAP_MAX)
	{
		struct marked * temp = LIST_FIRST(&list);
		LIST_FOREACH(temp,&list)
		{
			if(temp->va != vva) continue;
			sys_free_user_mem(temp->va,temp->pages*PAGE_SIZE);
			LIST_REMOVE(&list,temp);
			break;
		}
	}
	else panic("illegal access");
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	panic("smalloc() is not implemented yet...!!");
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
