/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
#include "event.h"
#include "read_handler.h"
#include "parser.h"
#include "io.h"
#include <errno.h>
#include <unistd.h>
#include <anet.h>
char master_cache[MAX_READ];
char slave_cache[MAX_READ];
char read_cache[MAX_READ];
extern int read_fd;
extern char *slave_ip;
extern int slave_port;
extern int master_fd;
extern int client_fd;
extern char *self_ip;
extern log4c_category_t* mycat;
extern int listen_fd;
extern int listen_port;
extern char *master_ip;
extern int master_port;

long cronloops = 0;

/**
 when connection with master fails, reconnect every 1 second.
**/
int timeEvent(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
	REDIS_NOTUSED(eventLoop);
	REDIS_NOTUSED(id);
	REDIS_NOTUSED(clientData);
	//if(cronloops == 1000)
	run_with_period(1000)
	{
		if(master_fd == -1){
			connectToMaster(eventLoop);
		}
	}
	cronloops++;
	return 1000/10;
}
/**
	read data from slave
**/
void readDataFromSlave(aeEventLoop *el, int fd, int flags)
{
	if(fd !=-1)
	{
		int ci;
		if ((ci =aeCreateFileEvent(el,fd,AE_READABLE,
            readFromSlave, slave_cache)) == AE_ERR)
		{
			log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "error create event readDataFromSlave error %d, at line %d in file %s",  errno,  __LINE__, __FILE__);
		 	closeFd(fd, el);
			return;
		}
		if(read_fd == -1)
		{
			read_fd  = anetTcpConnect(0, slave_ip, slave_port);
			if(read_fd == -1)
			{
				log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "read slave connection create fail errno %d,  ip %s, port %d, at line %d in file %s",  errno , slave_ip, slave_port,  __LINE__, __FILE__);
				closeFd(read_fd, el);
			}
			else
			{
				anetNonBlock(0, master_fd);
				anetEnableTcpNoDelay(0,read_fd);
				anetKeepAlive(0, read_fd, 10l);
				log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "read slave fd %d connection create success\n", read_fd);
				if(aeCreateFileEvent(el, read_fd, AE_READABLE, readDelValue, read_cache) == AE_ERR)
				{   
					log4c_category_log(mycat, LOG4C_PRIORITY_ERROR,  "error create file event  query key from slave.");
					closeFd(read_fd, el);
					return;
				}   
			}
		}
	}
}
/**
	accept slave connection
**/
void acceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask)
{
	REDIS_NOTUSED(el);
	REDIS_NOTUSED(mask);
	REDIS_NOTUSED(privdata);
	int cport, cfd;
	char *error = "";
	char cip[46];
	if(master_fd != -1){
		if(client_fd != -1)
		{
			close(client_fd);
		}
		cfd = anetTcpAccept(error, fd, cip, sizeof(cip), &cport);
		client_fd = cfd;
		anetKeepAlive(0, client_fd, 10l);
		if(cfd == ANET_ERR)
		{
			if(errno != EWOULDBLOCK)
			{
				printf("Accepting client connection: %d\n", errno);
				return;
			}
		}
		readDataFromSlave(el, cfd, 0);
	}
}
/**
 **	connect with redis master
 **/
int connectToMaster( aeEventLoop *eventLoop){
	errno = 0;
	master_fd  = anetTcpConnect(0, master_ip, master_port);
	if(master_fd != -1)
	{
		anetNonBlock(0, master_fd);
		if(aeCreateFileEvent(eventLoop, master_fd, AE_READABLE, readFromMaster, master_cache) == AE_ERR)
		{
			printf( "error create file event read query from client.");
			close(master_fd);
			return -1;
		}
		else{
			if(-1 == listenToPort(eventLoop)){
				exit(1);
			}
			return 1;
		}
	}
	else{
		printf("create maseter fd error %d\n", errno);
		return -1;
	}
	printf("master fd %d\n", master_fd);
	return 1;
}
/**
	listen to slave connection
**/
int listenToPort( aeEventLoop *loop)
{
	char error[256];
	listen_fd =  anetTcpServer(error, listen_port, self_ip, 1);
	if(listen_fd !=-1)
	{
		anetNonBlock(0, listen_fd);
		anetEnableTcpNoDelay(0,listen_fd);
		anetKeepAlive(0, listen_fd, 10l);
		if(aeCreateFileEvent(loop, listen_fd, AE_READABLE, acceptTcpHandler,0) == AE_ERR)
		{
			log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "error create event acceptTcpHandler error %d, at line %d in file %s",  errno,  __LINE__, __FILE__);
			close(listen_fd);
			return -1;
		}

	}
	else
	{
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "listen to port fd error %d, at line %d in file %s",  errno,  __LINE__, __FILE__);
		return -1;
	}
	return 1;
}
