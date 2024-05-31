/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//Handle the page fault

void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//cprintf("here");
#if USE_KHEAP
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(curenv->page_WS_list));
#else
		int iWS =curenv->page_last_WS_index;
		uint32 wsSize = env_page_ws_get_size(curenv);
#endif
if(isPageReplacmentAlgorithmFIFO()){

	if(wsSize < (curenv->page_WS_max_size))
		{

			//TODO: [PROJECT'23.MS2 - #15] [3] PAGE FAULT HANDLER - Placement
			//cprintf("placement\n");
		struct FrameInfo* aa;

	//	allocate_frame(&aa);


	//	int x= pf_read_env_page(curenv, &fault_va);


		allocate_frame(&aa);
		map_frame(curenv->env_page_directory,aa,fault_va,PERM_USER|PERM_WRITEABLE|PERM_PRESENT|PERM_AVAILABLE);
		int x= pf_read_env_page(curenv, (void*)fault_va);
		if(x!=0){
			if(fault_va>= USTACKBOTTOM && fault_va < USTACKTOP){


			}
			else if(fault_va>=USER_HEAP_START&&fault_va<USER_HEAP_MAX){

			}
			else{
				//cprintf("sabryyyy\n");
				sched_kill_env(curenv->env_id);

			}
		}
	struct WorkingSetElement *new_blk=env_page_ws_list_create_element(curenv,fault_va);
		LIST_INSERT_TAIL(&curenv->page_WS_list,new_blk);
		aa->ws_addr = (uint32)new_blk;
		//curenv->page_last_WS_element = LIST_NEXT(new_blk);
		if(LIST_SIZE(&curenv->page_WS_list)==curenv->page_WS_max_size){
			curenv->page_last_WS_element=LIST_FIRST(&curenv->page_WS_list);
		}
		else{
			curenv->page_last_WS_element=NULL;
		}

		}else{
			//fifo replacment

			uint32 va=curenv->page_last_WS_element->virtual_address;
			struct WorkingSetElement* temp1;
			int permissions = pt_get_page_permissions(curenv->env_page_directory, va);



								if(permissions&PERM_MODIFIED){
																struct FrameInfo* a;
																uint32 *ptr=NULL;
																a=get_frame_info(curenv->env_page_directory,va,&ptr);
																pf_update_env_page(curenv,va,a);
															}

										if(curenv->page_last_WS_element==LIST_LAST(&curenv->page_WS_list)){
											temp1=LIST_FIRST(&curenv->page_WS_list);
										}else{

										 temp1=LIST_NEXT(curenv->page_last_WS_element);
										}

										struct WorkingSetElement* temp2=curenv->page_last_WS_element;
										struct WorkingSetElement* block=env_page_ws_list_create_element(curenv, fault_va);
										struct FrameInfo* aa;
										allocate_frame(&aa);
									map_frame(curenv->env_page_directory,aa,fault_va,PERM_WRITEABLE|PERM_USER|PERM_PRESENT|PERM_MODIFIED);
					          if(temp1==LIST_FIRST(&curenv->page_WS_list)){
					       	struct WorkingSetElement*temp3=curenv->page_last_WS_element->prev_next_info.le_prev;
						    LIST_INSERT_AFTER(&curenv->page_WS_list,temp3,block);
						 LIST_REMOVE(&curenv->page_WS_list,temp2);
					      }else{
				         LIST_INSERT_BEFORE(&curenv->page_WS_list,temp1,block);
					   LIST_REMOVE(&curenv->page_WS_list,temp2);
					}

                              aa->ws_addr=(uint32)block;
					          unmap_frame(curenv->env_page_directory,va);
					          env_page_ws_invalidate(curenv,(curenv->page_last_WS_element->virtual_address));

						curenv->page_last_WS_element=temp1;
						int x= pf_read_env_page(curenv, (void*)fault_va);
								if(x!=0){
									if(fault_va>= USTACKBOTTOM && fault_va < USTACKTOP){


									}
									else if(fault_va>=USER_HEAP_START&&fault_va<USER_HEAP_MAX){

									}
									else{
										//cprintf("sabryyyy\n");
										sched_kill_env(curenv->env_id);

									}
								}



		}

}
if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))
{
//TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement
// Write your code here, remove the panic and write your code
//panic("page_fault_handler() LRU Replacement is not implemented yet...!!");

if(LIST_SIZE(&curenv->ActiveList)+LIST_SIZE(&curenv->SecondList)<curenv->page_WS_max_size){

//palcment here

struct FrameInfo* aa;

//int x= pf_read_env_page(curenv, &fault_va);

//		if(x!=0){
//			if(fault_va>= USTACKBOTTOM && fault_va < USTACKTOP){
//				allocate_frame(&aa);
//				map_frame(curenv->env_page_directory,aa,fault_va,PERM_USER|PERM_WRITEABLE|PERM_PRESENT|PERM_MODIFIED);
//
//
//
//			}
//			else if(fault_va>=USER_HEAP_START&&fault_va<USER_HEAP_MAX){
//				allocate_frame(&aa);
//				map_frame(curenv->env_page_directory,aa,fault_va,PERM_USER|PERM_WRITEABLE|PERM_PRESENT|PERM_MODIFIED);
//
//
//
//			}
//			else{
//				//cprintf("sabryyyy");
//
//				sched_kill_env(curenv->env_id);
//			}
//
//		}
//		else{
//
//
//
//
////				//refer to the project presentation and documentation for details
//		}
//	struct WorkingSetElement *ff;
//	LIST_FOREACH(ff,&curenv->ActiveList){
//
//					if(ff->virtual_address==fault_va){
//					LIST_REMOVE(&curenv->ActiveList,ff);
//					LIST_INSERT_HEAD(&curenv->ActiveList,ff);
//					return;
//
//					}
//				}



	struct WorkingSetElement *new_blk=env_page_ws_list_create_element(curenv,fault_va);

	if(LIST_SIZE(&curenv->ActiveList)<curenv->ActiveListSize){

		allocate_frame(&aa);
	   map_frame(curenv->env_page_directory,aa,fault_va,PERM_PRESENT|PERM_USER|PERM_WRITEABLE|PERM_AVAILABLE);

		LIST_INSERT_HEAD(&curenv->ActiveList,new_blk);

	}else{
		struct WorkingSetElement *i=NULL;
		struct WorkingSetElement *f;
		bool found=0;
		LIST_FOREACH(i,&curenv->SecondList){

			if(ROUNDDOWN(i->virtual_address,PAGE_SIZE)==ROUNDDOWN(fault_va,PAGE_SIZE)){
				found=1;
				LIST_REMOVE(&curenv->SecondList,i);
				pt_set_page_permissions(curenv->env_page_directory,i->virtual_address,PERM_PRESENT,0);
				f=i;

				break;

			}
		}
		if(found){
		//cprintf("ZOZA\n");
		struct WorkingSetElement *old_blk=LIST_LAST(&curenv->ActiveList);
							pt_set_page_permissions(curenv->env_page_directory,old_blk->virtual_address,0,PERM_PRESENT);
							LIST_REMOVE(&curenv->ActiveList,old_blk);
							LIST_INSERT_HEAD(&curenv->ActiveList,i);
							LIST_INSERT_HEAD(&curenv->SecondList,old_blk);



		}else{

		allocate_frame(&aa);
		map_frame(curenv->env_page_directory,aa,fault_va,PERM_PRESENT|PERM_USER|PERM_WRITEABLE|PERM_AVAILABLE);
		struct WorkingSetElement *old_blk=LIST_LAST(&curenv->ActiveList);
		pt_set_page_permissions(curenv->env_page_directory,old_blk->virtual_address,0,PERM_PRESENT);
		LIST_REMOVE(&curenv->ActiveList,old_blk);
		LIST_INSERT_HEAD(&curenv->SecondList,old_blk);
		LIST_INSERT_HEAD(&curenv->ActiveList,new_blk);
		}


	}


	int x= pf_read_env_page(curenv, (void*)fault_va);
			if(x!=0){
				if(fault_va>= USTACKBOTTOM && fault_va < USTACKTOP){


				}
				else if(fault_va>=USER_HEAP_START&&fault_va<USER_HEAP_MAX){

				}
				else{
					//cprintf("sabryyyy\n");
					sched_kill_env(curenv->env_id);

				}
			}



}else{
//replacment here


	 //cprintf("%x \n",fault_va);

	struct FrameInfo* aa;
	struct WorkingSetElement *i=NULL;
			struct WorkingSetElement *f;
			bool found=0;
			LIST_FOREACH(i,&curenv->SecondList){

				if(ROUNDDOWN(i->virtual_address,PAGE_SIZE)==ROUNDDOWN(fault_va,PAGE_SIZE)){
					found=1;
					LIST_REMOVE(&curenv->SecondList,i);
					pt_set_page_permissions(curenv->env_page_directory,i->virtual_address,PERM_PRESENT,0);
					f=i;

					break;

				}
			}
			if(found){
		//	cprintf("ZOZA\n");
			struct WorkingSetElement *old_blk=LIST_LAST(&curenv->ActiveList);
								pt_set_page_permissions(curenv->env_page_directory,old_blk->virtual_address,0,PERM_PRESENT);
								LIST_REMOVE(&curenv->ActiveList,old_blk);
								LIST_INSERT_HEAD(&curenv->ActiveList,i);
								LIST_INSERT_HEAD(&curenv->SecondList,old_blk);



			}else{

				struct WorkingSetElement *removed=LIST_LAST(&curenv->SecondList);
				int permissions = pt_get_page_permissions(curenv->env_page_directory, removed->virtual_address);
			if(permissions&PERM_MODIFIED){
					struct FrameInfo* a;
					uint32 *ptr=NULL;
					a=get_frame_info(curenv->env_page_directory,removed->virtual_address,&ptr);
					pf_update_env_page(curenv,removed->virtual_address,a);
						}


        LIST_REMOVE(&curenv->SecondList,removed);
        unmap_frame(curenv->env_page_directory,removed->virtual_address);
        env_page_ws_invalidate(curenv,(removed->virtual_address));
        struct WorkingSetElement *old_blk=LIST_LAST(&curenv->ActiveList);
		 pt_set_page_permissions(curenv->env_page_directory,old_blk->virtual_address,0,PERM_PRESENT);
		 allocate_frame(&aa);
		 map_frame(curenv->env_page_directory,aa,fault_va,PERM_PRESENT|PERM_USER|PERM_WRITEABLE|PERM_AVAILABLE);
		 struct WorkingSetElement *new_blk=env_page_ws_list_create_element(curenv,fault_va);
		  LIST_REMOVE(&curenv->ActiveList,old_blk);
		 LIST_INSERT_HEAD(&curenv->SecondList,old_blk);
		 LIST_INSERT_HEAD(&curenv->ActiveList,new_blk);
		 int x= pf_read_env_page(curenv, (void*)fault_va);
		 		if(x!=0){
		 			if(fault_va>= USTACKBOTTOM && fault_va < USTACKTOP){


		 			}
		 			else if(fault_va>=USER_HEAP_START&&fault_va<USER_HEAP_MAX){

		 			}
		 			else{
		 				//cprintf("sabryyyy\n");
		 				sched_kill_env(curenv->env_id);

		 			}
		 		}


















}
		//	int x= pf_read_env_page(curenv, (void*)fault_va);
			//env_page_ws_print(curenv);

//TODO: [PROJECT'23.MS3 - BONUS] [1] PAGE FAULT HANDLER - O(1) implementation of LRU replacement
}

}
}



void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	panic("this function is not required...!!");
}



