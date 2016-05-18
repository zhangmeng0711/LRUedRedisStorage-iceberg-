/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
#ifndef _READ_CONF_H_
#define _READ_CONF_H_
#include <glib.h>

typedef struct
{
  int listen_port;
  gchar *master_ip;
  gchar *self_ip;
  int master_port;
  gchar *slave_ip;
  gchar **ips;
  gsize ip_num;
  int slave_port;
} Settings;
Settings *conf;
Settings* load_conf (char *file_name);
#endif
