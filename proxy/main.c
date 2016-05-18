/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
#include "event.h"
#include "read_conf.h"
#include "util.h"
#include <assert.h>
#include <ae.h>
#include <errno.h>
#include <stdio.h>
#include "hash_table.h"
#include "basic.h"
#include <log4c.h>
/**
	open file descriptors. 
*/
int master_fd = -1;//as master role, to receive connection from client
int client_fd = -1;//as client role, to communicate with master
int type_fd = -1;
int read_fd = -1;// read key values tobe deleted from slave

char *self_ip; //proxy ip
int listen_port; //proxy listen to slave connection
char *slave_ip;
int slave_port;
char *master_ip;
int master_port;

long long time_event_id;

log4c_category_t* mycat;

//save lrued data to file system, suppose there are multi server to store the data and with backups
char **host_masters; //the master file system servers
int host_master_num;
hashtable_t *hashtable; // key is fs master server ip, value is fs slave ip, suppose 1 backup each fs master


struct WData buf_master_cmd; //buffer master to slave command
struct WData buf_slave_answer; //buffer slave answer to lrued key query

/**
	init file system storage server configuration
**/
int initHost(){
	int i;
	host_masters = (char*)calloc(conf->ip_num/2, 16*sizeof(char));
	for(i = 0; i < conf->ip_num; i += 2){
		host_master_num++;
		host_masters[i/2] = conf->ips[i];
	}
	
	//init fs backup map; 1--1
	hashtable = ht_create( 65536 );
    int ip_index = 0;
    for(ip_index = 0; ip_index < conf->ip_num; ip_index += 2){
		ht_set( hashtable, conf->ips[ip_index], conf->ips[ip_index + 1]);
	}
	return 1;
}



int main(int argc, char **argv)
{
	if(argc < 1)
	{
		printf("please input configure file");
		exit(1);
	}
	log4c_init();
	mycat = log4c_category_get("six13log.log.app.iceberg");

    if(!load_conf(argv[1])){
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "read conf failed\n");
		exit(1);
	}
	self_ip = conf->self_ip;
	log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "self ip  %s\n", self_ip);
    listen_port = conf->listen_port;
	log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "listen port %d\n", listen_port);
	master_ip = (char*)conf->master_ip;
	log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "master ip %s\n", master_ip);
    master_port = conf->master_port;
	log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "master port %d\n", master_port);
	slave_ip = (char*)conf->slave_ip;
	log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "slave ip %s\n", slave_ip);
    slave_port = conf->slave_port;
	log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "slave port %d\n", slave_port);

	char buf_master_cmd_oc[MAX_READ];
	buf_master_cmd.len = 0;
	buf_master_cmd.data = buf_master_cmd_oc;
	char buf_slave_answer_oc[MAX_READ];
	buf_slave_answer.len = 0;
	buf_slave_answer.data = buf_slave_answer_oc;

	if(initHost() == -1){
		printf("hash host init failed\n");
		return -1;
	}
	aeEventLoop* loop = aeCreateEventLoop(1024);

	if(connectToMaster(loop) == -1){
		exit(1);
	}

	//init queue
	init_cir_queue(&q);



	//curl init
	curlInit();

	if((time_event_id = aeCreateTimeEvent(loop, 1,timeEvent , 0, 0)) == AE_ERR)
	{
		printf( "Unrecoverable error creating time event.");
		return -1;
	}
	aeMain(loop);
	//curl_global_cleanup();
	aeDeleteEventLoop(loop);
	return 0;
}
