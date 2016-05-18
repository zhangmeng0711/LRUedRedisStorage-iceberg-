/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
#include "io.h"
#include <stdio.h>
#include <log4c.h>
#include <errno.h>
#include "event.h"
#include <unistd.h>
extern int master_fd;
extern log4c_category_t* mycat;
extern long long time_event_id;
extern int client_fd;
extern int listen_fd;
extern int read_fd;
extern int state;
extern struct WData buf_slave_answer;
extern struct WData buf_master_cmd;
cir_queue_t q;
struct WData merge_buf_query;

int listen_fd = -1;
int state = START_STATE;
int readData(aeEventLoop *el,  int fd, char *data, size_t size){
	if(master_fd == -1){
		connectToMaster(el);
	}
	if(fd == -1){
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "readData fd ==-1 at line %d in file %s", __LINE__, __FILE__);
		return -1;
	}
	int nread = read(fd, data, size);
	if(nread == 0){
		if(errno != EAGAIN  && errno != EINPROGRESS){
			log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "connection closed,  errno = %d at line %d in file %s",errno,  __LINE__, __FILE__);
			closeFd(fd, el);
			return -1;
		}
		return 0;
	}
	else if(nread < 0){
		if(errno != EAGAIN  && errno != EINPROGRESS ){
			log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "readData  errno = %d at line %d in file %s",errno,  __LINE__, __FILE__);
			closeFd(fd, el);
			return -1;
		}
		return 0;
	}
	else
		return nread;

}

int writeData( aeEventLoop* el,int fd, char *data, int len){
	int wr;
	int all_write = 0;
	if(master_fd == -1){
		connectToMaster(el);
	}
	if(fd == -1){
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "%s , at line %d in file %s", "write fd == -1 error ",  __LINE__, __FILE__);
		return -1;
	}
	while(all_write < len){
		wr = write(fd, data + all_write, len - all_write);
		if(wr <= 0){
			if(errno == 11 || errno == EINPROGRESS){
				continue;
			}
			else{
				log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "write fd %d all write %d, len %d, errno %d at line %d in file %s", fd, all_write, len, errno, __LINE__, __FILE__);
				closeFd(fd, el);
				return -1;
			}
		}
		else{
			all_write += wr;
			if(all_write < wr){
				continue;
			}
		}

	}
	return all_write;
}
void closeFd(int fd, aeEventLoop* el) {
	aeDeleteTimeEvent(el, time_event_id);
	if(client_fd != -1){
		aeDeleteFileEvent(el, client_fd, AE_READABLE);
		close(client_fd);
		client_fd = -1;
	}
	if(read_fd != -1){
		aeDeleteFileEvent(el, read_fd, AE_READABLE);
		close(read_fd);
		read_fd = -1;
	}
	if(master_fd != -1){
		aeDeleteFileEvent(el, master_fd, AE_READABLE);
		close(master_fd);
		master_fd = -1;
	}
	if (listen_fd != -1) {
		aeDeleteFileEvent(el, listen_fd, AE_READABLE);
		close(listen_fd);
		listen_fd = -1;
	}
	state = START_STATE;
	buf_master_cmd.len = 0;
	buf_slave_answer.len = 0;
	clear_queue(&q);
	if((time_event_id = aeCreateTimeEvent(el, 1,timeEvent , 0, 0)) == AE_ERR)
	{
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "Unrecoverable error creating time event errno = %d at line %d in file %s",errno,  __LINE__, __FILE__);
	}
}

