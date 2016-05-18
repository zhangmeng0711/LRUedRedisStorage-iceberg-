/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
 
#include "read_handler.h"
#include "io.h"
#include "basic.h"
#include "lrued_query.h"
#include "parser.h"
#include <errno.h>
#include <string.h>
extern int state;
extern int master_fd;
extern int client_fd;
extern log4c_category_t* mycat;
long long int bulk_size;//sync state payload read size
void readDelValue(aeEventLoop *el, int fd, void *privdata, int mask) {
    REDIS_NOTUSED(el);
    REDIS_NOTUSED(mask);
	char data[MAX_READ];
	int nread = readData(el, fd, data, MAX_READ);
	if(nread > 0){
		struct WData wdata;
		wdata.len = nread;
		wdata.data = data;
		parseReadData(el, fd, mask, &wdata);
	}
}
void readFromSlave(aeEventLoop *el, int fd, void *privdata, int mask){
	REDIS_NOTUSED(el);
	REDIS_NOTUSED(mask);
	char rdata[MAX_READ];
	int nread = readData(el, fd, rdata, MAX_READ);
	if(nread <= 0 )
	{
		return;
	}
	if(nread == 6){
		rdata[6]= 0;
		char *sync = "SYNC\r\n";
		if(strcmp(sync, rdata) == 0){
			log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "sync start");
			state = SYNC_START_STATE;
		}		
	}
	int wr = writeData(el, master_fd, rdata, nread);
	if(-1 == wr)
	{
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "write to master failed error %d, at line %d in file %s",  errno,  __LINE__, __FILE__);
	}
}
void readFromMaster(aeEventLoop *el, int fd, void *privdata, int mask) {
	REDIS_NOTUSED(el);
	REDIS_NOTUSED(mask);
	int nread;
	char rdata[MAX_READ];
	char *p = rdata;
	switch(state){
		case SYNC_START_STATE:{	 
			while(1){
				nread = readData(el, fd, p,1);
				if(nread < 0){
					return;
				}
				else if(nread == 0){
					continue;
				}
	
				int wr = writeData(el, client_fd, p, 1);
				if(-1 == wr)
				{
					return;
				}
				if(*p == '\n' && p != rdata) break;
				if(*p != '\n') p++;
			}
			*p = '\0';
			if(rdata[0] == '-'){
				log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "sync with master failed at line %d in file %s",   __LINE__, __FILE__);
				exit(1);
			}
			bulk_size = strtoull(rdata + 1, 0, 10);
			log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "bulk size %llu, at line %d in file %s",  bulk_size,  __LINE__, __FILE__);
			state = SYNC_STATE;
			break;
		} 
		case SYNC_STATE:{ 
			long long int all_read =0;
			long long int all_write =0;
			while(bulk_size){
				nread = readData(el, fd, rdata, (bulk_size > MAX_READ) ? MAX_READ:bulk_size);
				if(nread < 0 )
				{
					return;
				}
				else if (nread == 0){
					continue;
				}
				else{
					all_read += nread;
				}
				int wr = writeData(el, client_fd, rdata,nread);
				if(-1 == wr)
				{
					log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "write to payload failed error %d, at line %d in file %s",  errno,  __LINE__, __FILE__);
					return;
				}
				else{
					all_write += wr;
				}
				bulk_size -= nread;
			}
			state = RUNNING_STATE;		
			log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "sync done");
			break;
		} 
		default:{ 
			nread = readData(el, fd, rdata, MAX_READ);
			if(nread > 0){
				struct WData wdata;
				wdata.len = nread;
				wdata.data= rdata; 
				if( state == RUNNING_STATE){
					processInputBuffer(el, &wdata);
					int wr = writeData(el, client_fd, wdata.data, wdata.len);
					if(-1 == wr)
					{
						return;
					}
				}
				else{
					if(nread > 9 && nread < 100)
					{
						if(!strncmp(rdata,"+CONTINUE",9) || !strncmp(rdata,"+FULLRESYNC",11)){
		
							log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "sync start");
							state = SYNC_START_STATE;
						}
					}
					int wr = writeData(el, client_fd, wdata.data, wdata.len);
					if(-1 == wr)
					{
						return;
					}
		
				}
			}
			break; 
		} 
	}

	
}
