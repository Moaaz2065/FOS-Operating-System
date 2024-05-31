/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"
#include <inc/lib.h>


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
uint32 get_block_size(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->size ;
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->is_free ;
}

//===========================================
// 3) ALLOCATE BLOCK BASED ON GIVEN STRATEGY:
//===========================================
void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockMetaData* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", blk->size, blk->is_free) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//=========================================
	//DON'T CHANGE THESE LINES=================
	if (initSizeOfAllocatedSpace == 0)
		return ;
	is_initialized = 1;
	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()
	LIST_INIT(&Blocks);
	struct BlockMetaData *start;
	start=(struct BlockMetaData *)daStart;
	start->is_free=1;
	start->prev_next_info.le_next=(void*)NULL;
	start->prev_next_info.le_prev=(void*)NULL;
	start->size=initSizeOfAllocatedSpace;
	LIST_INSERT_TAIL(&Blocks,start);
}
//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================


void *alloc_block_FF(uint32 size)
{
	//
	//panic("alloc_block_FF is not implemented yet");


	if(size==0){
		return NULL;
	}
	if (!is_initialized)
	{


		uint32 required_size = size + sizeOfMetaData();
		uint32 da_start = (uint32)sbrk(required_size);


		//get new break since it's page aligned! thus, the size can be more than the required one
		uint32 da_break = (uint32)sbrk(0);
		initialize_dynamic_allocator(da_start, da_break - da_start);
	}
	struct BlockMetaData *block;

	LIST_FOREACH (block,&(Blocks)){
		if(!(block->is_free)){
			continue;
		}
		else if(block->size-sizeOfMetaData()>=size){
			if(block->size-sizeOfMetaData()==size){
				block->is_free=(1==2);
				return (struct BlockMetaData *)(block+1);
			}
			else if(block->size-sizeOfMetaData()>(size+sizeOfMetaData())){
				struct BlockMetaData *new_block;
				//assign new_block

				void* addres;
				addres=block;
				addres+=size+sizeOfMetaData();
				new_block=addres;
				new_block->size=block->size-(size+sizeOfMetaData());
				new_block->is_free=(1==1);
				//new_block->prev_next_info.le_next=block->prev_next_info.le_next;


				LIST_INSERT_AFTER(&(Blocks),block,new_block);
				 //modify block
				block->is_free=(1==2);
				block->size=size+sizeOfMetaData();
				return (struct BlockMetaData *)(block+1);
			}
			else{
				block->is_free=(1==2);

				return(struct BlockMetaData *) (block+1);
			}

		}

	}

	void *check = sbrk(size+sizeOfMetaData());
	if(check == NULL || check == (void *)-1)
		return NULL;
	if(sbrk(0)-check == size+sizeOfMetaData())
	{
		void * address = LIST_LAST(&Blocks);
		address += LIST_LAST(&Blocks)->size;
		struct BlockMetaData * sbrk_block;
		sbrk_block = address;
		sbrk_block->size = size+sizeOfMetaData();
		sbrk_block->is_free = 0;
		LIST_INSERT_TAIL(&Blocks,sbrk_block);
	}
	else if(sbrk(0)-check > size+sizeOfMetaData())
	{
		void * address = LIST_LAST(&Blocks);
			address += LIST_LAST(&Blocks)->size;
			struct BlockMetaData * sbrk_block;
			sbrk_block = address;
			sbrk_block->size = size+sizeOfMetaData();
			sbrk_block->is_free = 0;
			LIST_INSERT_TAIL(&Blocks,sbrk_block);
			void * addressElba2y = sbrk_block;
			addressElba2y += size+sizeOfMetaData();
			struct BlockMetaData * end;
			end = addressElba2y;
			end->size = ROUNDUP(size+sizeOfMetaData(),PAGE_SIZE)-size-sizeOfMetaData();
			end->is_free = 1;
			LIST_INSERT_TAIL(&Blocks,end);
			return sbrk_block+1;
	}
	else
	{
		void * address = LIST_LAST(&Blocks);
		address += LIST_LAST(&Blocks)->size;
		struct BlockMetaData * sbrk_block;
		sbrk_block = address;
		sbrk_block->size = (uint32)(sbrk(0)-check);
		sbrk_block->is_free = 0;
		LIST_INSERT_TAIL(&Blocks,sbrk_block);
		return sbrk_block+1;
	}
	return NULL;
}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
	struct BlockMetaData *block,*b_block=NULL;
	uint32 b_size=2e9;
	LIST_FOREACH (block,&(Blocks)){
		if(block->size-sizeOfMetaData()<size||block->is_free==(1==2)){
			continue;
		}
		if(block->size-sizeOfMetaData()-size<b_size){
			b_size=block->size-sizeOfMetaData()-size;
			b_block=block;
		}

	}
	if(b_block==NULL){
		void* z=sbrk(size+sizeOfMetaData());

		return NULL;
	}
	else if(b_block->size-sizeOfMetaData()==size){
		b_block->is_free=(1==2);
		return b_block+1;
	}
	else if(b_block->size-sizeOfMetaData()>size+sizeOfMetaData()){

		void* addres;
		struct BlockMetaData*new_block;
		addres=b_block;
		addres+=size+sizeOfMetaData();
		new_block=addres;
		new_block->size=b_block->size-(size+sizeOfMetaData());
		new_block->is_free=(1==1);
		LIST_INSERT_AFTER(&(Blocks),b_block,new_block);
		b_block->size=size+sizeOfMetaData();
		b_block->is_free=(1==2);
		return b_block+1;
	}
	else{
		b_block->is_free=(1==2);
		//b_block->size=size;
		return b_block+1;
	}
	//panic("alloc_block_BF is not implemented yet");

	return NULL;
}

//=========================================
// [6] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

//===================================================
// [8] FREE BLOCK WITH COALESCING:
//===================================================
uint32 get_block_up(struct BlockMetaData *blk)
{
	if(blk == NULL || blk->is_free == 0)
		return 0;

	uint32 ret = blk->size;

	ret += get_block_up(blk->prev_next_info.le_next);

	blk->size = 0;
	blk->is_free = 0;
	LIST_REMOVE(&(Blocks),blk);
	return ret;
}
struct BlockMetaData *get_block_down(struct BlockMetaData *blk)
{

	if(blk == LIST_FIRST(&(Blocks)) && blk->is_free)
		return blk;

	if(blk->is_free == 0)
		return blk->prev_next_info.le_next;

	struct BlockMetaData *ret = get_block_down(blk->prev_next_info.le_prev);

	if(ret == blk)
		return ret;

	ret->size += blk->size;
	blk->size = 0;
	blk->is_free = 0;
	LIST_REMOVE(&(Blocks),blk);
	return ret;
}
void free_block(void *va)
{
	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()

	struct BlockMetaData *blk = va;
	blk--;
	blk->is_free = (1==1);
	uint32 sz = 0;

	if(blk != LIST_LAST(&(Blocks)))
		sz += get_block_up(blk->prev_next_info.le_next);

	struct BlockMetaData *curr_bot = blk;

	if(blk != LIST_FIRST(&(Blocks)))
		curr_bot = get_block_down(blk->prev_next_info.le_prev);

	if(curr_bot != blk)
	{
		sz += blk->size;
		blk->size = 0;
		blk->is_free = 0;
		LIST_REMOVE(&(Blocks),blk);
	}

	curr_bot->size += sz;
	curr_bot->is_free = 1;
}
//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void Add_block(struct BlockMetaData *curr_blk,uint32 sz,uint32 new_size)
{
	void *ptr = curr_blk;
	ptr += new_size+sizeOfMetaData();
	struct BlockMetaData *blk = ptr;
	blk->size = sz-new_size-sizeOfMetaData();
	blk->is_free = (1==1);
	LIST_INSERT_AFTER(&(Blocks),curr_blk,blk);
}

void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()
	if(va == NULL && new_size == 0)
		return NULL;

	if(va == NULL)
		return alloc_block_FF(new_size);

	if(new_size == 0)
	{
		free_block(va);
		return NULL;
	}

	struct BlockMetaData *curr_blk = va;
	curr_blk--;
	int sz = curr_blk->size;

	if(sz-sizeOfMetaData() == new_size)
		return curr_blk+1;

	if(sz-sizeOfMetaData() > new_size)
	{
		if(sz > new_size+(sizeOfMetaData()<<1))
		{
			Add_block(curr_blk,curr_blk->size,new_size);
            curr_blk->size = new_size+sizeOfMetaData();
		}
		return curr_blk+1;
	}
	else
	{
		uint32 AddedSz = 0;
		if(curr_blk != LIST_LAST(&(Blocks)))
			AddedSz += get_block_up(curr_blk->prev_next_info.le_next);

		curr_blk->size += AddedSz;
		if(curr_blk->size > new_size+(sizeOfMetaData()<<1))
		{
			Add_block(curr_blk,curr_blk->size,new_size);
			curr_blk->size = new_size + sizeOfMetaData();
			return curr_blk+1;
		}
		else if(curr_blk->size-sizeOfMetaData() >= new_size)
		{
			return curr_blk+1;
		}
		else
		{
			curr_blk->is_free = (1==1);
			return alloc_block_FF(new_size);
		}
	}
}
