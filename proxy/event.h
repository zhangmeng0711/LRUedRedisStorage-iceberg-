/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
#ifndef _EVENT_H_
#define _EVENT_H_
#include <ae.h>
#define run_with_period(_ms_) if ((_ms_ <= 1000/10) || !(cronloops%((_ms_)/(1000/10))))

int timeEvent(struct aeEventLoop *eventLoop, long long id, void *clientData);
#endif