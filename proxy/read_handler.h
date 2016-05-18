/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
#ifndef _READ_HANDLER_H_
#define _READ_HANDLER_H_
#include <ae.h>
void readDelValue(aeEventLoop *el, int fd, void *privdata, int mask);
void readFromSlave(aeEventLoop *el, int fd, void *privdata, int mask);
void readFromMaster(aeEventLoop *el, int fd, void *privdata, int mask);
#endif
