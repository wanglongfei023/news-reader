/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/04/10 23:08:12
*	Description:
*
==========================================================================*/

#include <news_reader.h>

int analyze_url(url_t* u, int nReqType)
{
	if(u == NULL)
		return -1; 
	if(nReqType != _UPDATE_ALL_NEWS)
	{
		bzero(u->domain, sizeof(u->domain)); 
		bzero(u->ip, sizeof(u->ip)); 
		bzero(u->szFilePath, sizeof(u->szFilePath)); 
		bzero(u->path, sizeof(u->path)); 
	}

	int starlen; 
	char* star[]={"http://", "https://"}; 
	int i, j, size; 
	struct hostent* ent; 
	size = 0; 

	//1.确定http版本
	//确定端口80/443
	if(strncmp(u->url, star[0], strlen(star[0]))==0)
	{
		u->type = TRUE; 
		u->port = 80; 
		starlen = strlen(star[0]); 
	}else if(strncmp(u->url, star[1], strlen(star[1]))==0){
		u->type = FALSE; 
		u->port = 443; 
		starlen = strlen(star[1]); 
	}else{
		return -1; 
	}
	//2.拿取域名
	j = 0; 
	for(i = starlen; i < strlen(u->url) && u->url[i] != '/' && u->url[i] != '\0'; i++)
	{
		u->domain[j++] = u->url[i]; 
	}

	//3.获取文件名
	for( i = strlen(u->url); 
		 nReqType == _SEARCH_ONE_NEWS_REQUEST && i >= 0 && u->url[i] != '/'; 
		 i--, ++size
	   ); 
	
	j = 0; 
	for( 
		i= strlen(u->url) - size + 1; 
		nReqType == _SEARCH_ONE_NEWS_REQUEST && i < strlen(u->url) && u->url[i] != '\0'; 
		i++
		)
	{
		u->szFilePath[j++] = u->url[i]; 
	}

	//4.获取资源路径
	j = 0; 
	for(
		i = starlen + strlen(u->domain); 
		nReqType == _SEARCH_ONE_NEWS_REQUEST && i < (strlen(u->url) - size); 
		i++
		)
	{
		u->path[j++] = u->url[i]; 
	}

	//5.获得ip
	if((ent = gethostbyname(u->domain)) != NULL)
	{
		inet_ntop(AF_INET, ent->h_addr_list[0], u->ip, sizeof(u->ip)); 
	}else{	
		return -1; 
	}
	LOG("debug:[1] analize url success.");

	return 0; 

}


int create_connect_socket()
{
	int socketfd; 
	if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		LOG("warning: fail to create connect_web_server socket."); 
		return -1; 
	}
	LOG("debug: [2]create connetion socket success."); 

	return socketfd; 
}


int connect_web_server(url_t* u, int webfd)
{
	struct sockaddr_in serveraddr; 
	serveraddr.sin_family = AF_INET; 
	serveraddr.sin_port = htons(u->port); 
	inet_pton(AF_INET, u->ip, &serveraddr.sin_addr.s_addr); 

	if((connect(webfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr))) == 0)
	{
		LOG("debug: [3]connect webserver success.\n"); 
		return 0; 

	}else{
		LOG("debug: [3]connect webserver failed.\n"); 
		return -1; 
	}
}


int create_request_head(char* pHead, url_t* u)
{
	bzero(pHead, 4096); 
	sprintf(pHead, "GET %s HTTP/1.1\r\n"\
				  "Accept:txt/html, application/xhtml+xml, application/xml; q=0.9, image/webp, */*; q=0.8\r\n"\
				  "User-Agent:Mozilla/5.0 (x11;  Linux x86_64) ApplewebKit/537(KHTML,  like Gecko) Chrom/47.0.2526Safari/537.36\r\n"\
				  "Host:%s\r\n"\
				  "Connection:close\r\n\r\n", u->url, u->domain); 

	LOG("debug: [4]create request head success."); 
	return 0; 

}

int send_request_head(char* pHead, int webfd, ssl_t* ssl)
{

	if(ssl != NULL)
	{
		if((SSL_write(ssl->sslsocket, pHead, strlen(pHead))) > 0)
		{
			LOG("debug: [5]send request head with SSL to webserver success.\n"); 
			return 0; 
		}else{
			LOG("warning: [5]send request head with SSL to webserver failed.\n"); 
			return -1; 
		}

	}else{
		if((send(webfd, pHead, strlen(pHead), 0)) > 0)
		{
			LOG("debug: [5]send request head to webserver success.\n"); 
			return 0; 
		}else{
			LOG("warning: [5]send request head to webserver failed.\n"); 
			return -1; 
		}
	}

}


ssl_t* opensslcreate(int webfd)
{
	ssl_t* ssl = (ssl_t*)malloc(sizeof(ssl_t)); 
	//初始化所有openssl错误处理信息
	SSL_load_error_strings(); 
	//初始化所有oepnssl所有库
	SSL_library_init(); 
	//初始化加载加密散列函数
	OpenSSL_add_ssl_algorithms(); 
	//创建安全上下文信息
	ssl->sslctx = SSL_CTX_new(SSLv23_method()); 
	//通过安全上下文信息创建socket
	ssl->sslsocket = SSL_new(ssl->sslctx); 
	//使用原有的webfd对sslsocket进行设置
	SSL_set_fd(ssl->sslsocket, webfd); 
	//使sslsocket与服务器交互
	SSL_connect(ssl->sslsocket); 

	return ssl; 

}

int get_web_response(url_t* u, int webfd, ssl_t* ssl)
{
	char buf[8192]; 
	char res[4096]; 
	char* head_link; 
	char* head_bad; 
	char* head_ser; 
	bzero(buf, sizeof(buf)); 
	bzero(res, sizeof(res)); 
	char* pos; 
	int len; 
	int fd; 
	if(ssl != NULL)
	{
		len = SSL_read(ssl->sslsocket, buf, sizeof(buf)); 
		if((pos = strstr(buf, "\r\n\r\n")) != NULL)
		{
			snprintf(res, pos - buf, "%s", buf); 
		}

		head_link = strstr(res, "Location"); //???
		head_bad = strstr(res, "400"); 
		head_ser = strstr(res, "500"); 
		if(head_link != NULL || head_bad != NULL || head_ser != NULL)
		{

			return -1; 
		}else{
			fd = open(u->szFilePath, O_RDWR | O_CREAT, 0775); //内容写进文件 
			write(fd, pos + 4, len - ( pos - buf + 4)); 
			LOG("debug: [6]get response with ssl success.\n%s\n", res); 
			bzero(buf, sizeof(buf)); 
			len = 0; 
			while((len = SSL_read(ssl->sslsocket, buf, sizeof(buf))) > 0)
			{
				write(fd, buf, len); 
				bzero(buf, sizeof(buf)); 
			}
			close(webfd); 
			close(fd); 
			free(ssl); 
		}
	}else{
		len = recv(webfd, buf, sizeof(buf), 0); 
		if(len <= 0)
			return -1; 
		head_link = strstr(buf, "Location"); 
		if(head_link != NULL)
		{
			return -1; 
		}
		if((pos = strstr(buf, "\r\n\r\n")) != NULL)
		{
			snprintf(res, pos-buf, "%s", buf); 
		}
		
	
		fd = open(u->szFilePath, O_RDWR | O_CREAT, 0775); 
		
		write(fd, pos + 4, len - (pos - buf + 4)); 
		LOG("debug: [6]Get resopnse sccess."); 
		bzero(buf, sizeof(buf)); 
		while((len = read(webfd, buf, sizeof(buf))) > 0)
		{
			write(fd, buf, len); 
			bzero(buf, sizeof(buf)); 
		}

		close(webfd); 
		close(fd); 
	}
	return 0; 

}
