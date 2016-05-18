
#include "myhash.h"
int hash(char *src, int slot){
	MD5_CTX md5;
	MD5Init(&md5);         
	int i;
	//unsigned char encrypt[] ="abc";//21232f297a57a5a743894a0e4a801fc3
	unsigned char decrypt[16];    
	MD5Update(&md5,src,strlen((char *)src));
	MD5Final(&md5,decrypt);        
	long long total = 0;
	for(i=0;i<16;i++)
	{
		total += decrypt[i];
	//	printf("%02x",decrypt[i]);
	}
	int num = total %slot;
	return num;	
} 