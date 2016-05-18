/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
#ifndef _PARSER_H_
#define _PARSER_H_
#include <jansson.h>
#include <log4c.h>
#include "basic.h"

void saveToBuf(struct WData *wdata, int start, struct WData *buf);
void popHandle(json_t *object) ;
void parseReadData(aeEventLoop *el, int fd, int mask, struct WData *wdata);
#endif
