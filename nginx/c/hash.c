#include "conhash.h"
#include <stdio.h>
#include "hash.h"
#include<dirent.h>
#include<unistd.h>
#include <sys/stat.h>
#include "./myhash/myhash.h"
#include <errno.h>
char url[45];
struct conhash_s *conhash;
struct conhash_s *routehash;
#define HASH_SLOT 10
#define ROUTE_SLOT 2
struct node_s g_nodes[10];
struct node_s route_nodes[2];
int init_route(const char filename[100]){
        routehash = conhash_init(NULL);
        if(routehash)
        {
                int j = 0;
                conhash_set_node(&route_nodes[0], "10.50.8.73", 1);
                conhash_set_node(&route_nodes[1], "10.50.8.74", 1);
                j = 0;
                for(j = 0; j < ROUTE_SLOT; j++)
                {
                        conhash_add_node(routehash, &route_nodes[j]);
                }
                return 1;
        }
        else
        {
		return -1;
        }
}
const char* getHost(const char key[100], const char filename[100]){
        if(init_route(filename)){
                const struct node_s *node = conhash_lookup(routehash, key);
                if(node && node->iden){
                        conhash_fini(routehash);
                        return node->iden;
                }
                        conhash_fini(routehash);
        }
        return "error";
}
int init(){
	conhash = conhash_init(NULL);
	if(conhash)
	{
		int j = 0;
		char str[5];
		for(j = 0; j < HASH_SLOT; j++)
		{
			sprintf(str,"%d",j);
			conhash_set_node(&g_nodes[j], str, 1);
		}
		j = 0;
		for(j = 0; j < HASH_SLOT; j++)
		{
			conhash_add_node(conhash, &g_nodes[j]);
		}
		return 1;
	}
	else
	{
	//	log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "hash failed  at line %d in file %s",  __LINE__, __FILE__);
		return -1;
	}
}
const char*  writeGetPath(const char key[100], int slot){
	int num = hash(key, slot);
	int key_len =strlen(key);
	int index = 38 ;
	int cur = strlen(key);
	char *str = (char*)calloc(9 + cur, sizeof(char));
	strncpy(str, key, cur);
    	strncpy(url, "/usr/local/openresty/nginx/html/files/", index);
    	int anchor;
	int i = 0;
	while(i < 3){
		anchor = slot;
		while(anchor > 1){
			anchor = anchor/10;
			url[index++] = num/anchor + '0';
			str[cur++] = url[index-1];
			num = num%anchor;
		}
		url[index++] =  '/';
		    
		url[index] =  0;
		DIR *dir = NULL;
		dir = opendir(url);
		if(dir == NULL){
			int i = mkdir(url, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);	
			if(i != 0)
			{
				//log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "mkdir error %d  at line %d in file %s", errno,  __LINE__, __FILE__);
			}
		}
		else
			closedir(dir);
		num = hash(str, slot);
		i++;
	}
	url[index] = 0;
	free(str);
	//log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "url %s  at line %d in file %s", url,   __LINE__, __FILE__);
	return url;	
}

const char*  readGetPath(const char key[100], int slot){
	int num = hash(key, slot);
	int key_len =strlen(key);
	int index = 7 ;
	int cur = strlen(key);
	char *str = (char*)calloc(9 + cur, sizeof(char));
	strncpy(str, key, cur);
    	strncpy(url, "/files/", index);
    	int anchor;
	int i = 0;
	while(i < 3){
		anchor = slot;
		while(anchor > 1){
			anchor = anchor/10;
			url[index++] = num/anchor + '0';
			str[cur++] = url[index-1];
			num = num%anchor;
		}
		url[index++] =  '/';
		    
		num = hash(str, slot);
		i++;
	}
	url[index] = 0;
	free(str);
	return url;	
}

