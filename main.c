#include "config.h"
#include "malloc.h"
#include "memlib.h"

int main()
{
   int ret = mm_init(); 
   if (ret == -1) {
        printf("init error");
        return -1;
   }

   char * p = (char*)mm_malloc(100);
   memcpy(p, "helloworld", 100);
   
   printf("%s", p);

}
