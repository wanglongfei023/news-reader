/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/04/08 23:06:12
*	Description:
*
==========================================================================*/
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/socket.h>
#include<string.h>
#include<sys/epoll.h>
#include<regex.h>
#include<signal.h>
#include<errno.h>
#include<arpa/inet.h>
#include<sys/mman.h>
#include<netdb.h>
#include<time.h>
#include<openssl/ssl.h>
#include<openssl/err.h>
#include<pthread.h>
#include<ctype.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<signal.h>
#include<netinet/in.h>
#include <stdarg.h>

//#define SERVER_IP "192.168.43.2"
#define SERVER_IP 		"0.0.0.0"
#define SERVER_PORT 	8000
#define EPOLL_MAX  		1000
#define LISTEN_NUM		128
#define BUFF_SIZE 	 	1024
#define IP_SIZE  		16
#define HASH_LEN 		1000
#define TIMEOUT 		3600

#define TRUE			1
#define FALSE			0

#define SUCCESS 		1
#define FAILED 			0

#define _CURRENT_NEWS_REQUEST 			2
#define _SEARCH_ONE_NEWS_REQUEST 		3
#define _SEARCH_OLD_NEWS_REQUEST 		4
#define _VIDEO_NEWS_REQUEST 			5
#define _PICTURE_NEWS_REQUEST 			6
#define _UPDATE_ALL_NEWS 				159


#define LOG(...) write_log(__FILE__,  __func__,  __LINE__,  __VA_ARGS__)

FILE* pLogFileHandler; 
time_t pGlobalTimeFile; 
struct tm* tm_finame; 
char pGblCurTextFile[50]; 
char pGblCurJpgFile[50]; 
char pGblCurVedioFile[50]; 
char szGlbTodayTime[50]; 


typedef struct
{
	void* (*job)(void*); 
	void* arg; 
}task_t; 


typedef struct
{
	int thread_max; 
	int thread_min; 
	int thread_alive; 
	int thread_busy; 
	int queue_max; 
	int queue_cur; 
	int shut_down; 
	int killno; 
	int queue_front; 
	int queue_rear; 
	pthread_t *tids; 
	pthread_t managertid; 
	task_t* queue_task; 
	pthread_cond_t full; 
	pthread_cond_t empty; 
	pthread_mutex_t lock; 
	pthread_attr_t arr; 

}pool_t; 


typedef struct  			//通信所用结构体
{
	char buf[300]; 
	char url[300]; 
	int buflen; 
	int nPackType; 
	int clifd; 
}net_data_t; 


typedef struct 				 //服务器链接所需参数
{
	int ser_fd; 
	int epfd; 
	int* ep_num; 
}client_info_t; 


typedef struct 				//解析https所用的结构体
{
	SSL_CTX* sslctx; 
	SSL* sslsocket; 
}ssl_t; 


typedef struct    			//解析url所用到的结构体
{
	char url[4096]; 
	char domain[1024];   	//主机名
	char path[1024]; 
	char szFilePath[255]; 
	char ip[16]; 
	char title[1024]; 
	int port; 
	int type; 		 		//如果TRUE是http，FALSE为https
}url_t; 


typedef struct node 		//去重所用hash结构体
{
	char url[300]; 
	char szFilePath[300]; 
	time_t time_num; 
	struct node* pNext; 
	int hash_len; 

}hash_t; 


typedef struct delhash
{
	int num; 
	hash_t* arr[5]; 
}hash_del; 


pool_t* pGlobalPool;  		//线程池
hash_t** pGlobalHashTable;  //去重所用哈西结构指针



int init_log(const char*);											//初始化日志文件 
int write_log(const char*,  const char*,  int,  const char*,  ...); //打印日志
void drop_log(); 													//删除日志句柄
int create_socket(const char* ip, short host);   				    //初始化socket并进行绑定
int add_socket(int epfd, int fd);   						        //epoll的添加
int delete_socket(int epfd, int fd);   							    //epoll的删除
pool_t* thread_pool_create(int max, int min, int que_max); 	 		//创建线程池
int thread_pool_destory(pool_t* pool);  						    //线程池销毁
void* thread_task_add(pool_t*, void* (*)(void*), void*); 		    //生产者
void* thread_manager_func(void*);  								    //管理者线程函数
void* search_news_for_client(void*);  							    //为用户查询或爬取新闻
void* thread_worker_func(void*);     								//消费者线程函数
int if_thread_alive(pthread_t);  								    //判断线程是否存活
int create_connect_socket();     								    //创建用于url通信的socket
void* accept_client(client_info_t*); 								//服务器链接
int analyze_url(url_t*, int);   									//获取url_t信息
int connect_web_server(url_t*, int);   								//为获取url指向资源建立链接
int create_request_head(char*, url_t*);   							//创建请求头
ssl_t* opensslcreate(int);   										//获取安全的socket
int send_request_head(char*, int, ssl_t*);  						//向url指向服务器发送请求
int get_web_response(url_t*, int, ssl_t*);  						//获取url指向服务器发送的头
int regular_match(regex_t* recom, regmatch_t* regch, char* string, 
		          int n, FILE* filefd, int Renum);  				//服务器解析收到的包
int send_text_file(net_data_t* date, FILE* filefd);  				//向客户端发送文本文件
int send_binary_file(net_data_t*, FILE*); 							//发送二进制文件
int handle_request(pool_t*, net_data_t*);   						//请求处理入口
void* update_all_news(void*); 										//为系统文件更新信息
void* search_old_news(void*); 										//为客户端搜索历史新闻
void* push_client_current_news(void*);								//给客户端发送信息
void* deal_video_request(void*); 									//给客户端发送视频
void* deal_picture_request(void*);  							    //给客户端更新图片信息
hash_t** hash_init(int nLength);  									//哈西表初始化
int destroy_hash_table(hash_t**);   								//哈西表销毁
hash_t* hash_search(hash_t**, char* url); 							//非空为有该元素，空为没有该元素
int hash_push(hash_t**, char* url, char* szFilePath, time_t); 		//1为添加成功，0为添加失败
char* split_string(char* string, char* buf, int* nlen); 			//读取文件数据加载到哈西表中
int collect_string_into_hash(hash_t**, int, int); 					//读取文件数据加载到哈西表中
void* thread_time_update_func(void*); 								//定时更新文件数据

