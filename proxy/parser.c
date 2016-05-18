/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
 
#include <ae.h>
#include "store.h"
#include "myhash.h"
#include <string.h>
#include "parser.h"
extern struct WData merge_buf_query;
extern struct WData buf_slave_answer;
extern log4c_category_t* mycat;

/**
 * save incomplete command to buf_cmd/buf_slave_answer
 */

void saveToBuf(struct WData *wdata, int start, struct WData *buf){
	buf->len = wdata->len - start;
	if(buf->len < MAX_READ)
		strncpy(buf->data, wdata->data + start, buf->len);
	else{
		buf->len=0;
	}
}
/*
 * return current index;
 *  -1 incomplete command;
 */
int parseArray(struct WData *wdata, int start){
	int index = start;
	index++;
	if(index >= wdata->len){
		saveToBuf(wdata, start, &buf_slave_answer);
		return -1;
	}
	int array_len = 0;
	int enter = 0;
	while(wdata->data[index] != '\r' && index < wdata->len){
		array_len = array_len * 10 +  wdata->data[index] - '0';
		index++;
		enter = 1;
		 if(index >= wdata->len){
			saveToBuf(wdata, start, &buf_slave_answer);
			return -1;
		}
	}
	index += 2;
	if(index > wdata->len || (index == wdata->len && array_len > 0)){
		saveToBuf(wdata, start, &buf_slave_answer);
		return -1;
	}
	char *item_key;
	char *item_value;
	json_t *object = json_object();
	while(array_len > 0){
		if(wdata->data[index] != '$'){
			log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "unexpected data, index %d, start %d, wdata->len %d, index value %c at line %d in file %s, data %s",index, start, wdata->len, wdata->data[index],  __LINE__, __FILE__, wdata->data);
			printf("2unexpected data, index %d, start %d, wdata->len %d, index %d\n", index, start, wdata->len, wdata->data[index]);
			int i;
			for(i = start; i <= index; i++)
				printf("%c", wdata->data[i]);
			for(i = index + 1; i< wdata->len; i++)
				printf("%c", wdata->data[i]);
				
			exit(1);
		}
		index++;
		if(index >= wdata->len){
			saveToBuf(wdata, start, &buf_slave_answer);
			json_decref(object);
			return -1;
		}
		int item_len = 0;
		int item_start;
		while(wdata->data[index] != '\r' && index < wdata->len){
			item_len = item_len * 10 +  wdata->data[index] - '0';
			index++;
		}
		if(index >= wdata->len){
			saveToBuf(wdata, start, &buf_slave_answer);
			json_decref(object);
			return -1;
		}
		index++;
		item_start = index + 1;
		index += item_len + 2;
		index++;
		if(index > wdata->len || (index == wdata->len && array_len > 1)){
			saveToBuf(wdata, start, &buf_slave_answer);
			json_decref(object);
			return -1;
		}

		if(item_len > 0){
			if(array_len %2 == 0){
				item_key = (char*)calloc(item_len + 1, sizeof(char));
				strncpy(item_key, wdata->data + item_start, item_len);
			}
			else{
				item_value = (char*)calloc(item_len + 1, sizeof(char));
				strncpy(item_value, wdata->data + item_start, item_len);
				if(item_key && item_value){
					json_t *item_json;
					item_json = json_string(item_value);
					free(item_value);
					json_object_set(object, item_key, item_json);
					json_decref(item_json);
					free(item_key);
				}
				else{
					log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "item key is  null or item value is null  at line %d in file %s", __LINE__, __FILE__);
				}
			}
		}

		array_len--;
	}

	popHandle(object);
	return index;
}
/**
 * return -1 incomplete command;
 *  current index;
 */
int parseBulkString(struct WData *wdata, int start){
	log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "parse bulk string  at line %d in file %s, %s", __LINE__, __FILE__, wdata->data);
	int index = start;
	index++;
	int item_len = 0;
	int data_start;
	while(wdata->data[index] != '\r' && index < wdata->len){
		item_len = item_len * 10 +  wdata->data[index] - '0';
		index++;
	}
	if(index >= wdata->len){
		saveToBuf(wdata, start, &buf_slave_answer);
		return -1;
	}
	index++;
	data_start = index + 1;
	index += item_len + 2;
	index++;
	if(index > wdata->len){
		saveToBuf(wdata, start, &buf_slave_answer);
		return -1;
	}
	char *saveData = (char*)calloc(item_len + 1, sizeof(char));
	strncpy(saveData, wdata->data+ data_start, item_len);
	if(saveData == 0){
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "post data empty  at line %d in file %s", __LINE__, __FILE__);
	}
	popHandle(saveData);
	return index;
}
/*
 * return -1 incomplete command
 */
int parseSimple(struct WData *wdata, int start){
	int index = start;
	index++;
	int error_start = index;
	int exist = 0;
	while( index < wdata->len){
		if(wdata->data[index] == '\n'){
			exist = 1;
			break;
		}
		index++;
	}
	if(exist == 1){
		char *cmd = (char*)calloc(index - start + 1, sizeof(char));
		strncpy(cmd, &wdata->data[error_start], index + 1 - start);
		if(wdata->data[index] == '-' ){
			DataType e = pop_cir_queue(&q);
			log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "pop %s receive error %s  at line %d in file %s", e, cmd,  __LINE__, __FILE__);
		}
		else{
			if(index - start > 5){
				DataType e = pop_cir_queue(&q);
				log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "pop %s receive string %s  at line %d in file %s", e, cmd,  __LINE__, __FILE__);
			}
		}
		free(cmd);
		return index + 1;
	}
	else{
		saveToBuf(wdata, start, &buf_slave_answer);
		return -1;
	}
}
/*
 * dequeue data, match key value
 */
void popHandle(json_t *object) {
	 DataType e = pop_cir_queue(&q);
	 if(e == 0)
	 {
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "queue is empty at line %d in file %s",  __LINE__, __FILE__);
		exit(1);
		return;
	 }
	int saveRes = saveValue(object, e);
	if(saveRes == -1){
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "saveValue %s error at line %d in file %s",e ,  __LINE__, __FILE__);
		return;
	}
}
/**
	parse response from slave. There are two connections between proxy and slave, one to transfer 
	slave to master data, the other for proxy to query slave and get responses. This function 
	deals with the latter situation.
**/
void parseReadData(aeEventLoop *el, int fd, int mask, struct WData *wdata){
    int index = 0;
    int flag = 0;
    struct WData *processData = wdata;
    if(buf_slave_answer.len > 0){
		if(buf_slave_answer.len + wdata->len < MAX_READ * 2){
			struct WData merge_buf_cmd;
			flag = 1;
    		merge_buf_cmd.len=buf_slave_answer.len + wdata->len ;
			merge_buf_cmd.data=(char*) calloc(merge_buf_cmd.len, sizeof(char));
			strncpy(merge_buf_cmd.data, buf_slave_answer.data, buf_slave_answer.len);
			strncpy(merge_buf_cmd.data + buf_slave_answer.len, wdata->data , wdata->len );
			processData = &merge_buf_cmd;
		}
		buf_slave_answer.len=0;
     }
     while(index >= 0 && index < processData->len){
		if(processData->data[index] == '*'){
			index = parseArray(processData, index);
		}
		else if(processData->data[index] == '$'){
			index = parseBulkString(processData, index);
		}
		else if(processData->data[index] == '-' || processData->data[index] == '+' || processData->data[index] == ':'){
			index = parseSimple(processData, index);
		}
		else{
			log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "query data unexpected  index %d, total %d  at line %d in file %s, data %s",   index, processData->len,  __LINE__, __FILE__, processData->data);
			printf("query data unexpected index %d, total %d\n", index, processData->len);
			int i;
			for(i = 0; i < index; i++)
				printf("%c", processData->data[i]);
			printf("\n index %d\n", processData->data[index]);
			for(i = index + 1; i < processData->len; i++){
				printf("%c", processData->data[i]);
			}
			printf(" end\n");
			break;
		}
    }
 	if(flag){
    	free(processData->data);
    }
}
