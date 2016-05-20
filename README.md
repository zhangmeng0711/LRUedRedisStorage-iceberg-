
[中文文档](CREADME.md)
## Description
* when using redis as a storage, in order to keep the entire dataset we need to spare extra memory. Ordinarily, scaling redis means adding memory, although there was a vm solution, it was abandoned in later version due to performance decrease. Under certain circumstances, stored data can be classified into cold data and hot data. If cold data’s reading performance need is not so harsh, to store cold data and hot data separately can be an economic alternative. Our tool “iceberg” is such a solution to persist cold data to disk. According to redis configuration, when memory usage reach certain value, some data may be abandoned. Iceberg is intended to change this by store the abandoned data to disk and provide a rest access to them. In this way not only memory is saved, but also data integrity is preserved. Besides, our protocol solution can effectively avoid binding to certain redis version.

## Features


* Due to business requirements, only hash map lru is supported in current version.
* Cold data stored in file system is mapped to three level directories, with key as file name, map json as file content.
* To guarantee high performance, mishit redis query file system directly with a client side hash to certain ip and path, so automatically horizontal scaling is not supported.
* The key solution is a proxy between master and slave, because it cannot detect topological change cause by master slave failover, no switch function is supported currently.
* At the nginx server side, all ips and path is hard coded in hash.c file. To be fixed.
* When slave restarts, before data is loaded to memory, if a query occurs for lrued data, they will get nothing. So there may be a chance to lose some data in a very small time window.
* Tested on redis2.8 and redis cluster. But when using redis cluster, master slave failover can cause our problems, so choices must be made very carefully according to network conditions. We finally chose redis2.8 and ensure master, proxy and slaves running on 1 host.
* Code data and hot data auto switch is not supported, users read and write redis all by themselves. All data written into redis is regarded as hot data until they are lrued.
* All file system server must have 1 back up for the moment. Tobe corrected.

## result

* In real use, data is stored both in redis and file system, so performance is decided by both. The higher proportion the former, the better. We store map in redis, each size vary from several KBs to tens of KBs. The file system servers with 2 masters and 2 slaves, query only from them can reach 6900/qps, and 2.1ms per response time on average.

## Architecture

* We add a proxy between each master with its one slave, the proxy delay commands send to slave to query deleted values and write them to file system. Query to nginx server is sent in case of mishit redis. The delayed slaves can also server read requests when 100% accuracy is not required real time. We depend on file system’s own features to ensure random access for small files. The architecture is shown in the picture below.
![](pic/icebergStructure.jpg)


## proxy

* transfer data between redis master and slave，persist lrued data to disk.
* proxy dependency
	1.	glib2.0: path is specified in Makefile，you may change -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -lglib-2.0
	2.	Jansson:https://github.com/akheron/jansson
	3.	log4c:http://log4c.sourceforge.net/


## nginx server

* openresty provide file read and write operation for lrued data.
* enter c directory and run sh run.sh to generate libhash.so and put it in some place that openresty can recognize，all dependencies needed and nginx.conf  is under nginx directory

## client

Client provides a url for each key that mishit redis. So client, nginx server and proxy muster keeyp the configuration in consistency


