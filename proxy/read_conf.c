/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
#include "read_conf.h"
#include <glib.h>
Settings* load_conf ( char *file_name)
{
  GKeyFile *keyfile;
  GKeyFileFlags flags;
  GError *error = NULL;
  gsize length;
  
  /* Create a new GKeyFile object and a bitwise list of flags. */
  keyfile = g_key_file_new ();
  flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;
  
  /* Load the GKeyFile from keyfile.conf or return. */
  if (!g_key_file_load_from_file (keyfile, file_name , flags, &error))
  {
    g_error (error->message);
    return NULL;
  }
  
  /* Create a new Settings object. If you are using GTK+ 2.8 or below, you should
   * use g_new() or g_malloc() instead! */
  conf = g_slice_new (Settings);
  
  conf->listen_port = g_key_file_get_integer (keyfile, "iceberg", "listen_port", NULL);
  if(!conf->listen_port){
	return NULL;
  }
  conf->self_ip = g_key_file_get_string (keyfile, "iceberg", "self_ip", NULL);
  if(!conf->self_ip){
	return NULL;
  }
  conf->master_ip = g_key_file_get_string(keyfile, "iceberg", "master_ip", NULL);
  if(!conf->master_ip){
	
	return NULL;
  }
  conf->master_port = g_key_file_get_integer (keyfile, "iceberg", "master_port", NULL);
  if(!conf->master_port){
	return NULL;
  }
  conf->slave_ip = g_key_file_get_string(keyfile, "iceberg", "slave_ip", NULL);
  if(!conf->slave_ip){
	return NULL;
  }
  conf->slave_port = g_key_file_get_integer (keyfile, "iceberg", "slave_port", NULL);
  if(!conf->slave_port){
	return NULL;
  }
  conf->ips = g_key_file_get_string_list(keyfile, "iceberg","ips", &length, NULL);
  if(length % 2 != 0 || !conf->ips){
	return NULL;
  }
  conf->ip_num = length;
  return conf;
}
