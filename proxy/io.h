/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
#ifndef _IO_H_
#define _IO_H_
#include <stdio.h>
#include <ae.h>
#include "basic.h"
#define START_STATE 0
#define SYNC_START_STATE 1
#define RUNNING_STATE 3
#define SYNC_STATE 2

int readData(aeEventLoop *el,  int fd, char *data, size_t size);
int writeData( aeEventLoop* el,int fd, char *data, int len);
void closeFd(int fd, aeEventLoop* el);

#endif

