/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/05/12 09:03:20
*	Description: 处理用户点击url产生的新闻请求事件
*
==========================================================================*/

#include<news_reader.h>

void* search_news_for_client(void* arg)
{
	net_data_t* pReqNetData = (net_data_t*)arg; 
	char szFileName[64] = {0}; 
	
	url_t u; 
	strcpy(u.url, pReqNetData->url); 
	strcpy(pReqNetData->buf, "客户端接收一条数据"); 
	pReqNetData->nPackType = _SEARCH_ONE_NEWS_REQUEST; 
	FILE* pFileHandler = NULL; 
	char head[4096] = {0}; 
	LOG("debug: \n\n\n\nurl:%s\n\n\n", pReqNetData->url);

	if(analyze_url(&u, _SEARCH_ONE_NEWS_REQUEST) == -1)
		return NULL; 

	sprintf(szFileName, "Txt/%s", u.szFilePath); 
	pFileHandler = fopen(szFileName, "rb"); 

	if(pFileHandler != NULL)
	{
		send_text_file(pReqNetData, pFileHandler); 
		fclose(pFileHandler); 
		return 0; 
	}


	int webfd = create_connect_socket(); 

	if(connect_web_server(&u, webfd) == -1)
	{
		LOG("warning: fail to connect web server for searching.");
		return NULL; 
	}

	create_request_head(head, &u); 
	send_request_head(head, webfd, NULL); 
	if(get_web_response(&u, webfd, NULL) == -1)
	{
		LOG("warning: fail to get web response.");
		return NULL; 
	}

	regex_t recom; 
	regex_t racom; 
	regmatch_t ragch[2]; 
	regmatch_t regch[2]; 


	int fd = open(u.szFilePath, O_RDWR); 
	int len = lseek(fd, 0, SEEK_END); 
	char* string = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0); 
	char* tt = string; 
	char* ss = string; 

	regcomp(&racom, "<title>\\([^<]\\+\\?\\)</title>\\?", REG_ICASE);   //获取标题
	regcomp(&recom, "<p>\\([^<]\\+\\?\\)</p>\\?", REG_ICASE);           //获取内容
	
	pFileHandler = fopen(szFileName, "a+"); 

	int m = 0; 
	int n = 0; 
	int i = 0; 
	

	m =  regular_match(&racom, ragch, tt, m, pFileHandler, 2); 
	fwrite("\n", 1, strlen("\n"), pFileHandler); 

	while(i < 50)
	{
		n =  regular_match(&recom, regch, tt, n, pFileHandler, 2); 
		if(n == -1)
			break; 
		i++; 
	}

	n = 0; 

	if(i == 0)
	{
		regfree(&recom); 
		regcomp(&recom, "<p style=\"text-indent: 2em; \">\\([^<]\\+\\?\\)</p>\\?", REG_ICASE);   //获取内容
		while(i < 50)
		{
			n = regular_match(&recom, regch, ss, n, pFileHandler, 2); 
			if(n == -1)
				break; 
			i++; 
		}
	}
	
	fclose(pFileHandler); 
	pFileHandler = fopen(szFileName, "rb"); 	
	send_text_file(pReqNetData, pFileHandler); 

	regfree(&recom); 
	regfree(&racom); 
	munmap(string, len); 
	fclose(pFileHandler); 
	close(fd); 
	remove(u.szFilePath); 
	return 0; 
}
