/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
#include "io.h"
#include "basic.h"
#include "parser.h"
#include <string.h>
#include <errno.h>
extern int read_fd;
extern log4c_category_t* mycat;
extern struct WData buf_master_cmd;
/**
	send hgetall command to slave
*/
void queryKey(aeEventLoop* el, int len, char *key){
	if(!is_full_cir_queue(&q)){
		int wr;
		char *cmd = (char*)calloc( 8 + 2 + len + 1, sizeof(char));
		strncpy(cmd, "hgetall ", 8);
		strncpy(cmd + 8, key, len);
		strncpy(cmd + 8 + len, "\r\n", 2);
		if((wr = writeData(el, read_fd,cmd, 8+2+len)) == -1){
			log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "write query key error %d, at line %d in file %s",  errno ,  __LINE__, __FILE__);
		}
		else{
			push_cir_queue(&q, key);
		}
		free(cmd);
	}
	else{
		
		log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "queue is full  len %d, at line %d in file %s",  q.count,  __LINE__, __FILE__);
	}
}
/*
	parser del command, send a query to slave 
 * return -1 imcomplete command
 * 		num > 0 finish position
 */
int delParser(aeEventLoop* el,struct WData *wdata, int start){
	int index = start +  13;
	int len = 0;
	if(wdata->data[index] == '$'){
		index++;
		if(index >= wdata->len){
			saveToBuf(wdata, start, &buf_master_cmd);

			return -1;
		}
		while(wdata->data[index] != '\r'){
			len = len *10 + wdata->data[index] - '0';
			index++;
			if(index >= wdata->len){
				saveToBuf(wdata, start, &buf_master_cmd);
				return -1;
			}
		}
		index += 2;
		if(index >= wdata->len){
			saveToBuf(wdata, start, &buf_master_cmd);
			return -1;
		}
		if(index + len <= wdata->len){
			char *key = (char*)calloc(len + 1,sizeof(char));
			strncpy(key, wdata->data + index, len);
			queryKey(el, len, key);

		}
		else{
			saveToBuf(wdata, start, &buf_master_cmd);
		}
		return index + len + 2;
	}
	else{
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "not $ start %s at line %d in file %s",wdata->data + index,  __LINE__, __FILE__);
		return -1;
	}
}
/**
process master to slave command, capture del command
**/
void processInputBuffer(aeEventLoop* el,struct WData *wdata){
        int index = 0;
        int flag = 0;
        struct WData *processData = wdata;
        if(buf_master_cmd.len > 0){
                if( buf_master_cmd.len +  wdata->len  < MAX_READ * 2){
                		struct WData merge_buf_cmd;
                		flag = 1;
                		merge_buf_cmd.len=buf_master_cmd.len +  wdata->len;
						merge_buf_cmd.data=(char*) calloc(merge_buf_cmd.len, sizeof(char));
                        strncpy(merge_buf_cmd.data, buf_master_cmd.data, buf_master_cmd.len);
                        strncpy(merge_buf_cmd.data + buf_master_cmd.len, wdata->data, wdata->len);
                        processData = &merge_buf_cmd;
                }
                buf_master_cmd.len = 0;
        }

        while(index < processData->len){
                while(processData->data[index] != '*' && index < processData->len){
                        index++;
                }
                if(index < processData->len){
                        if(processData->len - 14 > 0){
                                char *cmp1 = "*2\r\n$3\r\nDEL\r\n$";
                                char cmp[14];
                                strncpy(cmp, processData->data + index, 14);
                                if(strcmp(cmp, cmp1) == 0){
                                        index = delParser(el, processData, index);
                                        if(index == -1){
                                                break;
                                        }
                                }
                                else{
                                        index += 14;
                                }
                        }
                        else{
                                saveToBuf(processData, index-1, &buf_master_cmd);
                                break;
                        }

                }
        }
        if(flag){
        	free(processData->data);
        }
}