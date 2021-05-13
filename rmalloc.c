#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include "rmalloc.h" 

rm_header rm_free_list = { 0x0, 0 } ;
rm_header rm_used_list = { 0x0, 0 };
void* point = 0x0;
void * rmalloc (size_t s) 
{
	
	if(s>getpagesize()-sizeof(rm_header)) return 0x0;

	rm_header_ptr itr_free = &rm_free_list;
	rm_header_ptr itr_used =&rm_used_list;
	rm_header_ptr dest, temp;
	size_t max_temp=0;
	int found_checker = 0;

	while(itr_free->next!=0x0){
			if((itr_free->next)->size >= s && (itr_free->next)->size > max_temp){
			max_temp = (itr_free->next)->size;
			found_checker = 1;
			temp = itr_free;
			dest = itr_free->next;
		}
		itr_free = itr_free->next;
	}

	if(found_checker==0){
		printf("%p\n",itr_free->next);
		itr_free->next =  mmap(point, getpagesize(), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
		point = itr_free->next+(getpagesize())/sizeof(rm_header);
		printf("ww %p %p\n",itr_free->next,point);
		itr_free->next->size = getpagesize()-sizeof(rm_header);
		itr_free->next->next = 0x0;
		temp = itr_free;
		dest = itr_free->next;
	}

	while(itr_used->next!=0x0){
		itr_used = itr_used->next;
	}
	
	if(s==dest->size){
		itr_used->next = dest;
		dest = dest->next;
		dest->next=0x0;
		return dest;
	}

	rm_header_ptr remain = dest+(sizeof(rm_header)+s)/sizeof(rm_header);
	remain->size = dest->size - s - sizeof(rm_header);
	remain->next = dest->next;
	itr_used->next = dest;
	dest->size = s;
	dest->next = 0x0;
	temp->next = remain;
	return itr_used->next;
}

void rfree (void * p) 
{
	rm_header_ptr dest = p;
	rm_header_ptr itr_used = &rm_used_list;
	rm_header_ptr itr_free = &rm_free_list;
	while(1){
		if(itr_used->next == dest){
			itr_used->next = dest->next;
			break;
		}
		itr_used=itr_used->next;
	}
	while(1){
		if(dest < rm_free_list.next){
			
			if(dest+(dest->size+sizeof(rm_header))/sizeof(rm_header)==rm_free_list.next){
				dest->size += sizeof(rm_header)+rm_free_list.next->size;
				dest->next = rm_free_list.next->next;
				rm_free_list.next = dest;	
				printf("sdf");
			}
			else{
				dest->next = rm_free_list.next;
				rm_free_list.next = dest;
			}
			break;
		}
		if(itr_free->next == 0x0 && dest > itr_free){
			dest->next = 0x0;
			itr_free->next = dest;
			if(dest == itr_free+(itr_free->size+sizeof(rm_header))/sizeof(rm_header)){
				itr_free->size += sizeof(rm_header)+dest->size;
				itr_free->next = dest->next;
			}
			break;
		}
		if(dest>itr_free && dest < itr_free->next){
			if(dest+(dest->size+sizeof(rm_header))/sizeof(rm_header)==itr_free->next){
				dest->size += sizeof(rm_header)+itr_free->next->size;
				dest->next = itr_free->next->next;	
				itr_free->next = dest;
			}
			else{
				dest->next = itr_free->next;
				itr_free->next = dest;
			}
			itr_free->next = dest;
			if(dest == itr_free+(itr_free->size+sizeof(rm_header))/sizeof(rm_header)){
				itr_free->size += sizeof(rm_header)+dest->size;
				itr_free->next = dest->next;
			}
			break;
		}
		itr_free = itr_free->next;
	}
	
}

void * rrealloc (void * p, size_t s) 
{
	// TODO
	return 0x0 ; // erase this 
}

void rmshrink () 
{
	// TODO
}

void rmconfig (rm_option opt) 
{
	// TODO
}


void 
rmprint () 
{
	rm_header_ptr itr ;
	int i ;

	printf("==================== rm_free_list ====================\n") ;
	for (itr = rm_free_list.next, i = 0 ; itr != 0x0 ; itr = itr->next, i++) {
		printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(rm_header), (int) itr->size) ;

		int j ;
		char * s = ((char *) itr) + sizeof(rm_header) ;
		for (j = 0 ; j < (itr->size >= 8 ? 8 : itr->size) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("\n") ;

	printf("==================== rm_used_list ====================\n") ;
	for (itr = rm_used_list.next, i = 0 ; itr != 0x0 ; itr = itr->next, i++) {
		printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(rm_header), (int) itr->size) ;

		int j ;
		char * s = ((char *) itr) + sizeof(rm_header) ;
		for (j = 0 ; j < (itr->size >= 8 ? 8 : itr->size) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("\n") ;

}
