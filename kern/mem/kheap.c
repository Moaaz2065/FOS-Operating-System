#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"


int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{

	initSizeToAllocate=ROUNDUP(initSizeToAllocate,PAGE_SIZE);
	start=daStart;
	brk=daStart+initSizeToAllocate;
	h_limit=daLimit;


	if(daStart+initSizeToAllocate>daLimit)
			return  E_NO_MEM;
	if((initSizeToAllocate/PAGE_SIZE)>LIST_SIZE(&free_frame_list))
		return  E_NO_MEM;


	for(uint32 i=start;i<(start+initSizeToAllocate);i+=PAGE_SIZE){
		struct FrameInfo* ptr_frame_info;

			allocate_frame(&ptr_frame_info);
			ptr_frame_info->va=i;
			map_frame(ptr_page_directory,ptr_frame_info, i,PERM_WRITEABLE|PERM_PRESENT|PERM_MODIFIED);

	}


	initialize_dynamic_allocator(start,initSizeToAllocate);




	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
	//All pages in the given range should be allocated
	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
	//Return:
	//	On success: 0
	//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM

	//Comment the following line(s) before start coding...
	return 0;
}

void* sbrk(int increment)
{
	//cprintf("%d \n",increment);

	//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
	/* increment > 0: move the segment break of the kernel to increase the size of its heap,
	 * 				you should allocate pages and map them into the kernel virtual address space as necessary,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * increment = 0: just return the current position of the segment break
	 * increment < 0: move the segment break of the kernel to decrease the size of its heap,
	 * 				you should deallocate pages that no longer contain part of the heap as necessary.
	 * 				and returns the address of the new break (i.e. the end of the current heap space).
	 *
	 * NOTES:
	 * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
	 * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
	 * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING====
	if(increment>0)
	{
		int inc=ROUNDUP(increment,PAGE_SIZE);


		uint32 old_brk=brk;
		if(ROUNDUP(brk,PAGE_SIZE)-brk >= increment)
		{
			brk+=inc;
			//cprintf("%x\n",old_brk);
			return (void *)old_brk;
		}
		uint32 sz=inc+brk;
		brk=ROUNDUP(brk,PAGE_SIZE);
		//old_brk=ROUNDUP(old_brk,PAGE_SIZE);
		for(uint32 i=brk;i<brk+increment;i+=PAGE_SIZE){
         struct FrameInfo* ptr_frame_info;

			if(LIST_SIZE(&free_frame_list)==0)
						panic("no free frames");
			if(inc+brk>h_limit){
						panic("exceeded hard limit");
					}
			int ret=allocate_frame(&ptr_frame_info);


			ptr_frame_info->va=i;
		    map_frame(ptr_page_directory, ptr_frame_info, i, PERM_WRITEABLE);
		}

		brk+=inc;
		return (void*)old_brk;
	}
	else if(increment<0)
	{
		 increment=increment*-1;
		if(brk-increment < KERNEL_HEAP_START)
			panic("exceeded heap start");
		if(increment>PAGE_SIZE){
			int k=increment/PAGE_SIZE;
			brk-=(k*PAGE_SIZE);
			for(uint32 i=brk;i>=brk-(k*PAGE_SIZE);i-=PAGE_SIZE){
				uint32 check=ROUNDUP(i,PAGE_SIZE);
						struct FrameInfo*aa;
						uint32* page;
						aa=get_frame_info(ptr_page_directory,check,&page);
						if(aa!=NULL){
							unmap_frame(ptr_page_directory,check);
						}

			}
			increment=increment%PAGE_SIZE;
		}
		brk-=increment;

		uint32 check=ROUNDUP(brk,PAGE_SIZE);
		struct FrameInfo*aa;
		uint32* page;
		aa=get_frame_info(ptr_page_directory,check,&page);
		if(aa!=NULL){
			unmap_frame(ptr_page_directory,check);
		}
		return (void*)brk;
	}
	else
	{
		//cprintf("%x\n",brk);
	void* x=(void*)brk;
		return x;
	}
	return NULL;
	//panic("not implemented yet");
}


void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy
	if(size <= DYN_ALLOC_MAX_BLOCK_SIZE){

		if(isKHeapPlacementStrategyFIRSTFIT())
			return alloc_block_FF((uint32)size);

		if(isKHeapPlacementStrategyBESTFIT())
			return alloc_block_BF((uint32)size);
	}

	uint32 page_sz = ROUNDUP(size,PAGE_SIZE);

	if(page_sz/PAGE_SIZE > LIST_SIZE(&(free_frame_list)))
		return NULL;
	uint32 st = h_limit + PAGE_SIZE;
	uint32 x=h_limit+PAGE_SIZE;
	uint32 pva;
	while(x<KERNEL_HEAP_MAX){

		uint32 *ptr_page_table;

		struct FrameInfo* aa=get_frame_info(ptr_page_directory,x,&ptr_page_table);
		if(aa!=NULL){
			x+=PAGE_SIZE;
			continue;
		}

		if(page_sz>KERNEL_HEAP_MAX-(x))
						return NULL;
		int counter=0;
		int p_counter=page_sz/PAGE_SIZE;
		bool enough_space=0;
		for(uint32 i=x;i<KERNEL_HEAP_MAX;i+=PAGE_SIZE){
			uint32* page;
			struct FrameInfo* aa=get_frame_info(ptr_page_directory,i,&page);
			if(aa==NULL){
				if(counter==0)
					pva=i;
				counter++;
			}
			else{
				counter=0;
				continue;
			}
			if(counter==p_counter){
				//pva=i;
				enough_space=1;
				break;
			}
		}
if(enough_space==0){
	return NULL;
}
		//pva=(void*)x;
		uint32 sz=(uint32)pva+page_sz;

	for(uint32 i = (uint32)pva ; i <sz ; i+=PAGE_SIZE)
	{
		struct FrameInfo *ptr;
		allocate_frame(&ptr);
		ptr->va=i;
		map_frame(ptr_page_directory,ptr,(i),PERM_WRITEABLE|PERM_PRESENT);
//		if(i==pva){
//			uint32* page;
//		struct FrameInfo*aa3=get_frame_info(ptr_page_directory,x,&page);
//		aa3->n=page_sz/PAGE_SIZE;
//		}

	}
	uint32* page;
	struct FrameInfo*aa3=get_frame_info(ptr_page_directory,pva,&page);
	aa3->n=page_sz/PAGE_SIZE;

	break;
	}
	//change this "return" according to your answer
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	return (void*)pva;
}
void kfree(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()

	uint32 va=(uint32)virtual_address;
	if(va<brk&&va>=start)
		return free_block((void*)va);

	else if(va>=(h_limit+PAGE_SIZE)&&va<KERNEL_HEAP_MAX){
		uint32* page;

		struct FrameInfo* aa=get_frame_info(ptr_page_directory,va,&page);

		int z=aa->n;


		for(uint32 i=0;i<z;i++){

			uint32* ptr_page_table;
			struct FrameInfo* aa=get_frame_info(ptr_page_directory,va+(i*PAGE_SIZE),&page);


			unmap_frame(ptr_page_directory,va+(i*PAGE_SIZE));

			//struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory, va+(i*4096), &ptr_page_table);






			//free_frame(ptr_frame_info);
			//ptr_page_table[PTX(va+(i*4096))]=0;
		}






	}





}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
//	panic("kheap_virtual_address() is not implemented yet...!!");


to_frame_info(physical_address);
	struct FrameInfo* aa=to_frame_info(physical_address);


	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

//int x=(int)physical_address/PAGE_SIZE;


//cprintf("%x\n",aa->va);
	if(aa->references==0)
		return 0;
return (aa->va&0xfffff000)+(physical_address&0x00000fff);

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{

	unsigned int pageno=virtual_address/PAGE_SIZE;
	uint32 *ptr_page_table;

get_page_table(ptr_page_directory,virtual_address,&ptr_page_table);

return (ptr_page_table[PTX(virtual_address)] & 0xFFFFF000)+(virtual_address & 0x00000FFF);
	//change this "return" according to your answer
	return 0;
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
	if(virtual_address==NULL)
		return kmalloc(new_size);
	else if(new_size==0){
				kfree(virtual_address);
		return NULL;}
	else{
		if(virtual_address>=(void*)KERNEL_HEAP_START&&virtual_address<(void*)KERNEL_HEAP_MAX){
			if(new_size>DYN_ALLOC_MAX_BLOCK_SIZE){
				void* new_address=kmalloc(new_size);
				if(new_address==NULL)
					return NULL;
				kfree(virtual_address);
				return new_address;
			}
			else{
				return realloc_block_FF(virtual_address,new_size);
			}


		}
		else{
			if(new_size<=DYN_ALLOC_MAX_BLOCK_SIZE){
				void* new_address=alloc_block_FF(new_size);
				if(new_address==NULL)
					return NULL;
			}
			else{
				new_size=ROUNDUP(new_size,PAGE_SIZE);
				struct FrameInfo* ptr;
				uint32* page;
				ptr=get_frame_info(ptr_page_directory,(uint32)virtual_address,&page);
				uint32 awlna=ptr->n*(PAGE_SIZE)+(uint32)virtual_address;
				uint32 a5rna=ptr->va+new_size;
				if(a5rna>KERNEL_HEAP_MAX){
					void* new_address=kmalloc(new_size);
					if(new_address==NULL)
						return NULL;
					kfree(virtual_address);
					return new_address;
							}
				for(uint32 i=awlna;i<a5rna;i+=PAGE_SIZE){

					struct FrameInfo* ptr2;
					ptr2=get_frame_info(ptr_page_directory,(uint32)virtual_address,&page);
					if(ptr2!=NULL){
						void* new_address=kmalloc(new_size);
						if(new_address==NULL)
							return NULL;
						kfree(virtual_address);
						return new_address;
											}
				}

					for(uint32 i=awlna;i<a5rna;i+=PAGE_SIZE){

						struct FrameInfo* ptr2;
						allocate_frame(&ptr2);
						map_frame(ptr_page_directory,ptr2,i,PERM_WRITEABLE|PERM_PRESENT);
						ptr2->va=i;
					}
					ptr->n=new_size/PAGE_SIZE;







			}
		}
	}
	return NULL;

}

