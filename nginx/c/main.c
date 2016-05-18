#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "hash.h"
long getCurrentTime()
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
int main(int argc,char *argv[]){
    printf("key %s\n", argv[1]);
    printf("c/c++ program:%ld\n",getCurrentTime());
    char* path = readGetPath(argv[1], 1000);
    printf("c/c++ program:%ld\n",getCurrentTime());
    printf("query host:%s\n", path);
    printf("c/c++ program:%ld\n",getCurrentTime());
    char* host = getHost(argv[1], "ips");
    printf("host %s\n", host);
    //int res = system("ls /usr/local/openresty/");
    //printf("res %s\n");
    return 0;
}

