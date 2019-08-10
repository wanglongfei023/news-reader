/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/04/27 12:08:58
*	Description:
*
==========================================================================*/

#include<news_reader.h>


int create_socket(const char* pIP, short nPort)
{
	if(pIP == NULL)
		return -1; 

	struct sockaddr_in serverAddr; 
	serverAddr.sin_family = AF_INET; 
	serverAddr.sin_addr.s_addr = inet_addr(pIP); 
	serverAddr.sin_port = htons(nPort); 

	int fd = socket(AF_INET, SOCK_STREAM, 0); 

	//设定断开连接后不进入2MSL状态
	int on = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
	{
		LOG("warning: fail to set socket as no double MSL.");
	}

	if(bind(fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		LOG("failed to create socket."); 
		return -1; 
	}
	return fd; 
}

void* accept_client(client_info_t* pClientInfo)
{
	char ip[IP_SIZE]; 
	struct sockaddr_in clientAddr; 
	int clientAddrLen = sizeof(clientAddr); 

	int fd = accept(pClientInfo->ser_fd, (struct sockaddr*)&clientAddr, (socklen_t*)&clientAddrLen); 
	inet_ntop(AF_INET, (struct sockaddr*)&clientAddr.sin_addr.s_addr, ip, IP_SIZE); 

	++*(pClientInfo->ep_num); 
	time_t t; 
	
	time(&t); 
	LOG("debug: time:%s\tclient ip:%s\taccept All client num:%d\n", 
		ctime(&t), 
		inet_ntop(AF_INET, (struct sockaddr*)&clientAddr.sin_addr.s_addr, ip, 16), 
		*(pClientInfo->ep_num)
		); 
	
	if(fd == -1)
	{
		LOG("warning: fail to accept a client.");
		return NULL; 
	}

	if(add_socket(pClientInfo->epfd, fd) == -1)
	{
		LOG("warning: fail to accept a client.");
		return NULL; 
	}

	return NULL; 
}

int add_socket(int epfd, int fd)
{

	struct epoll_event epen; 
	epen.data.fd = fd; 
	epen.events = EPOLLIN | EPOLLET; 

	if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &epen) == -1)
	{
		LOG("fail to add a new client socket."); 
		return -1; 
	}
	return 1; 
}


int delete_socket(int epfd, int fd)
{
	if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1)
	{
		LOG("fail to delete a client socket."); 
		return -1; 
	}
	return 1; 
}


