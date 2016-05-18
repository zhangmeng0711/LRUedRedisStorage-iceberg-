/*
 * Copyright (C) zhangmeng
 * Copyright (C) ifeng.com
 */
#include <curl/curl.h>
#include "hash_table.h"
#include <log4c.h>
#include "store.h"
extern char **host_masters;
extern int host_master_num;
extern hashtable_t *hashtable;
extern log4c_category_t* mycat;
void curlInit(){
	curl_global_init(CURL_GLOBAL_ALL);
}
int saveValue(json_t *object, char *key)
{
	if(!object)
	{
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "%s at line %d in file %s", "value is empty",  __LINE__, __FILE__);
		exit(1);
	}
	char *value = json_dumps(object,0);

	if(value == 0){
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "%s at line %d in file %s", "post date empty",  __LINE__, __FILE__);
		return -1;
	}
	char *host = host_masters[hash(key, host_master_num)];
	if( host)
	{
		sendToHost(host, key, value);
		char *backup = ht_get( hashtable, host );
		if(backup){
			sendToHost(backup, key, value);
		}
	}
	else
	{
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "%s at line %d in file %s", "hash error",  __LINE__, __FILE__);
		
	}

	free(value);
	free(key);
	json_decref(object);
	return 1;
}
void sendToHost(const char* host, const char* key, const char* value) {
	CURL* curl;
	curl = curl_easy_init();
	struct curl_slist* list = 0;
	if (curl) {
		char *encoded_key = curl_easy_escape(curl, key, 0);

		int key_len = strlen(encoded_key);
		int host_len = strlen(host);
		char *url = (char*) calloc(key_len + 7 + host_len + 20 + 6 + 3 + 1,
				sizeof(char));
		strncpy(url, "http://", 7);
		int index = 7;
		strncpy(url + index, host, host_len);
		index += host_len;
		strncpy(url + index, ":8090/writeData?key=", 20);
		index += 20;
		strncpy(url + index, encoded_key, key_len);
		list = curl_slist_append(list, "connection:close");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, value);
		curl_easy_setopt(curl, CURLOPT_USERAGENT,
				"Mozilla/5.0 (Windows NT 5.1; rv:19.0) Gecko/20100101 Firefox/19.0");
		CURLcode res;
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "%s, %s at line %d in file %s",host, curl_easy_strerror(res),  __LINE__, __FILE__);
		}
		free(url);
		curl_slist_free_all(list);
		curl_easy_cleanup(curl);
		free(encoded_key);
	} else {
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "%s at line %d in file %s", "curl init error",  __LINE__, __FILE__);
	}
}