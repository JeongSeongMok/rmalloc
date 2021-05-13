#include <unistd.h>
#include <stdio.h>
struct _rm_header {
	struct _rm_header * next ;
	size_t size ;
} ;

typedef struct _rm_header 	rm_header ;
typedef struct _rm_header *	rm_header_ptr ;
int main(){
    
  
    rm_header a;
    a.size=2;
    rm_header c;
    c.size=3;
    
    a.next = &c;

    rm_header p;
    p.size=1;
    
    rm_header_ptr temp = &p+5;
    
    temp->size = 5;
    
    printf("%ld",temp->size);
    return 0;
}