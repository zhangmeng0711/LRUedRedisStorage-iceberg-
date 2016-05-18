/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
#ifndef _STORE_H_
#define _STORE_H_
#include <jansson.h>
#include <log4c.h>
void curlInit();
int saveValue(json_t *object, char *key);
void sendToHost(const char* host, const char* key, const char* value);
#endif