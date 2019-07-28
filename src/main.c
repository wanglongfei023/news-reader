/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/04/24 17:07:21
*	Description: 服务器入口程序
*
==========================================================================*/

#include <news_reader.h>

int main()
{
	if(chdir("../") < 0)
	{
		exit(-1); 
	}

	init_log("./log/news_reader.log"); 

	pGlobalPool = thread_pool_create(1000,  10,  1000); 
	if(pGlobalPool == NULL)
	{
		LOG("create thread pool failed."); 
	}

	pGlobalHashTable = hash_init(HASH_LEN); 
	if(pGlobalHashTable == NULL)
	{
		LOG("hash table create failed."); 
		exit(-1); 
	}

	memset(pGblCurVedioFile, '\0', sizeof(pGblCurVedioFile)); 
	sprintf(pGblCurVedioFile, "Title/Video%d%d%d.txt", 1900, 1, 1); 
	pthread_t updateTid; 
	pthread_create(&updateTid, &pGlobalPool->arr, thread_time_update_func, NULL); 

	while(TRUE)
	{
		int listenfd = -1; 
		int nReadyNum; 
		int ep_num = 0; 
		
		listenfd = create_socket(SERVER_IP, SERVER_PORT); 
		if(listenfd == -1) 
			exit(-1); 

		listen(listenfd, LISTEN_NUM); 
		
		struct epoll_event ep_even[EPOLL_MAX]; 
		int epfd = epoll_create(EPOLL_MAX); 

		if(add_socket(epfd, listenfd) == -1) exit(-1); 
		int i = 0; 
		while(TRUE)
		{
			nReadyNum = epoll_wait(epfd, ep_even, EPOLL_MAX, -1); 
			for(i = 0; i < nReadyNum; i++)
			{
				if(ep_even[i].data.fd == listenfd && ep_even[i].events == EPOLLIN)
				{
					if(ep_num < EPOLL_MAX)
					{
						client_info_t clientInfo; 
						clientInfo.ser_fd = listenfd; 
						clientInfo.epfd = epfd; 
						clientInfo.ep_num = &ep_num; 

						accept_client(&clientInfo); 

						LOG(" debug: now the number of connecting is %d.", ep_num);
						
					}else{
						LOG("warning: connection is more than support."); 
						continue; 
					}
				}else if(ep_even[i].events == EPOLLIN){
					net_data_t netData; 
					int read_num = read(ep_even[i].data.fd, &netData, sizeof(net_data_t)); 
					while(read_num < 0 && errno == EINTR)
					{	
						read_num = read(ep_even[i].data.fd, &netData, sizeof(net_data_t)); 
					}

					if(read_num == 0 || read_num < 0)
					{	
						int clifd = ep_even[i].data.fd; 
						delete_socket(epfd, ep_even[i].data.fd); 
						close(clifd); 
						ep_num--; 
						continue; 
					}

					netData.clifd = ep_even[i].data.fd; 
					handle_request(pGlobalPool, &netData); 	
				}
			}
		}
	}
	thread_pool_destory(pGlobalPool); 	
	drop_log();

	return 0; 
}
