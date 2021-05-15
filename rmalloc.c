#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include "rmalloc.h" 

rm_header rm_free_list = { 0x0, 0 };
rm_header rm_used_list = { 0x0, 0 };

void* point = 0x0;
rm_option curr_opt = FirstFit;

void * rmalloc (size_t s) 
{
	int remain_byte;
	if(s%sizeof(rm_header)==0) remain_byte=0;
	else remain_byte = sizeof(rm_header)-s%sizeof(rm_header);
	rm_header_ptr itr_free = &rm_free_list;
	rm_header_ptr itr_used = &rm_used_list;
	rm_header_ptr dest, temp;
	size_t max_temp=0 , min_temp = -1;
	int found_checker = 0;

	size_t space_size = s+remain_byte;
	while(itr_free->next!=0x0){
		if(curr_opt == WorstFit){
			if((itr_free->next)->size >= s && (itr_free->next)->size > max_temp){
				max_temp = (itr_free->next)->size;
				found_checker = 1;
				temp = itr_free;
				dest = itr_free->next;
			}
			itr_free = itr_free->next;
		}
		else if(curr_opt = BestFit){
			if((itr_free->next)->size >= s && ((itr_free->next)->size < min_temp || min_temp==-1)){
				min_temp = (itr_free->next)->size;
				found_checker = 1;
				temp = itr_free;
				dest = itr_free->next;
			}
			itr_free = itr_free->next;
		}
		else{
			if((itr_free->next)->size >= s){	
				found_checker = 1;
				temp = itr_free;
				dest = itr_free->next;
				break;
			}
			itr_free = itr_free->next;
			
		}
	}

	if(found_checker==0 ){
		int remain_size = s;
		while(remain_size>0){
			if(itr_free->next==0x0) {
				itr_free->next =  mmap(point, getpagesize(), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
				point = itr_free->next+(getpagesize())/sizeof(rm_header);
				itr_free->next->size = getpagesize()-sizeof(rm_header);
			}
			else {
				mmap(point, getpagesize(), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
				point += (getpagesize())/sizeof(rm_header);
				itr_free->next->size += getpagesize();
			}
			remain_size -= (getpagesize()-sizeof(rm_header));
		}
		itr_free->next->next = 0x0;
		temp = itr_free;
		dest = itr_free->next;
	}

	while(itr_used->next!=0x0){
		itr_used = itr_used->next;
	}
	if(s==dest->size){
		temp->next=dest->next;
		dest->next=0x0;
		itr_used->next=dest;
		dest+=1;
		return dest;
	}
	void *new_point=(void*)dest+s+sizeof(rm_header);
	rm_header_ptr remain = (rm_header_ptr)new_point;
	remain->size = dest->size - s - sizeof(rm_header);
	remain->next = dest->next;
	
	itr_used->next = dest;
	itr_used->next->size =s;
	itr_used->next->next = NULL;
	temp->next = remain;
	dest+=1;
	return dest;
}

void rfree (void * p) 
{
	rm_header_ptr dest = p-sizeof(rm_header);
	void *dest_ptr = (void*) dest;
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
		void* itr_free_ptr = (void*)itr_free;
		if(dest_ptr < (void*)rm_free_list.next){
			if(dest_ptr+dest->size+sizeof(rm_header)==(void*)rm_free_list.next){
				dest->size += sizeof(rm_header)+rm_free_list.next->size;
				dest->next = rm_free_list.next->next;
				rm_free_list.next = dest;	
			}
			else{
				dest->next = rm_free_list.next;
				rm_free_list.next = dest;
			}
			break;
		}
		
		if(itr_free->next == 0x0 && dest_ptr > itr_free_ptr){
			dest->next = 0x0;
			itr_free->next = dest;
			if(dest_ptr == itr_free_ptr+itr_free->size+sizeof(rm_header)){
				itr_free->size += sizeof(rm_header)+dest->size;
				itr_free->next = dest->next;
			}
			break;
		}
		if(dest_ptr>itr_free_ptr && dest_ptr < (void*)itr_free->next){
			if(dest_ptr+dest->size+sizeof(rm_header)==(void*)itr_free->next){
				dest->size += sizeof(rm_header)+itr_free->next->size;
				dest->next = itr_free->next->next;	
				itr_free->next = dest;
			}
			else{
				dest->next = itr_free->next;
				itr_free->next = dest;
			}			
			if(dest_ptr == itr_free_ptr+itr_free->size+sizeof(rm_header)){
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
	rm_header_ptr dest = (rm_header_ptr)p;
	if(dest->size!=s){
		rfree(p);
		
		rm_header_ptr new_point = rmalloc(s);
		rm_header_ptr curr = new_point;

		size_t bound;
		if(dest->size>s) bound=s/sizeof(rm_header);
		else bound = dest->size/sizeof(rm_header);

		while(1){
			if(curr>new_point+s/sizeof(rm_header) || curr>new_point+dest->size/sizeof(rm_header)) break;
			*curr = *dest;
			curr++;
			dest++;
		}
		return new_point;
	}
	else{
		return dest;
	}
	
}

void rmshrink () 
{
	rm_header_ptr curr = rm_free_list.next;
	while(curr!=0x0){
		rm_header_ptr next = curr->next;
		void* curr_ptr = (void*)curr;
		munmap(curr_ptr, curr->size+sizeof(rm_header));
		curr=next;
	}
	rm_free_list.next = 0x0;
}

void rmconfig (rm_option opt) 
{
	curr_opt = opt;
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
