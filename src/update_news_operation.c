/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/05/08 14:03:56
*	Description:
*
==========================================================================*/

#include<news_reader.h>

void* update_all_news(void* arg)
{	
	url_t u; 
	net_data_t* pNetData = (net_data_t*)arg; 
	net_data_t* pReqNetData = NULL; 

	if(arg != NULL)
	{
		pReqNetData = (net_data_t*)malloc(sizeof(net_data_t)); 
	}

	char szPageFileName[64] = {0}; 
	unsigned int nTid = (unsigned int)pthread_self(); 
	sprintf(szPageFileName, "%x", nTid); 

	if(arg != NULL)
	{
		pReqNetData->nPackType = _UPDATE_ALL_NEWS; 
		pReqNetData->clifd = pNetData->clifd; 
		pReqNetData->nBuffLen = pNetData->nBuffLen; 
		strcpy(pReqNetData->url, pNetData->url); 
		strcpy(u.url, pReqNetData->url); 
		strcpy(u.szFilePath, szPageFileName); 
	}else{
		strcpy(u.url, "http://www.people.com.cn"); 
		printf("client url:%s\n", u.url); 
		strcpy(u.szFilePath, szPageFileName); 
	}

	char head[4096]; 
	bzero(head, sizeof(head)); 

	if(analyze_url(&u, _UPDATE_ALL_NEWS) == -1)
	{
		return NULL; 
	}

	int webfd = create_connect_socket(); 
	if(connect_web_server(&u, webfd) == -1)
	{
		LOG("warning: connect web server failed.");
		return NULL; 
	}

	create_request_head(head, &u); 
	send_request_head(head, webfd, NULL); 

	if(get_web_response(&u, webfd, NULL) == -1)
	{
		LOG("warning: failed to get response from web server."); 
		return NULL; 
	}

	regex_t recom; 
	regex_t racom; 
	regex_t rycom; 
	regex_t rxcom; 
	regex_t rzcom; 
	regex_t rrcom; 

	regmatch_t ragch[4]; 
	regmatch_t regch[4]; 
	regmatch_t rygch[3]; 
	regmatch_t rxgch[3]; 
	regmatch_t rzgch[3]; 
	regmatch_t rrgch[3]; 

	int fd = open(u.szFilePath, O_RDWR); 
	
	int len = lseek(fd, 0, SEEK_END); 
	printf("\nlen:%d\n", len); 
	char* string = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0); 
	char* atem = string; 
	char* ytem = string; 
	char* xtem = string; 
	char* ztem = string; 
	char* rtem = string; 
	char* strtem = string; 
	

	regcomp(&recom, "<li><a href=\"\\([^\"]\\+\\?\\.html\\)\" target=\"_blank\"><img src=\"\\([^\"]\\+\\?\\)\"[^<]\\+\\?</a><div class=\"show\"><a href=[^<]\\+\\?target='_blank'>\\([^<]\\+\\?\\)</a></div>", REG_ICASE); //人民1网站2图片3标题

	regcomp(&racom, "<a href=\"\\([^\"]\\+\\?\\.html\\)\" target=\"_blank\"><img src=\"\\([^\"]\\+\\?\\)\"[^<]\\+\\?</a><span><a href=[^<]\\+\\?target=\"_blank\">\\([^<]\\+\\?\\)</a></span>", REG_ICASE);  //人民网1网站2图片3标题

	regcomp(&rycom, "<li><a href=\"\\([^>]\\+\\?\\.html\\)\">\\([^<]\\+\\?\\)</a></li>", REG_ICASE); 

	regcomp(&rxcom, "<p><a href=\'\\([^>]\\+\\?\\.html\\)\' target=\"_blank\">\\([^<]\\+\\?\\)</a></p>", REG_ICASE); //人民1网站2标题

	regcomp(&rzcom, "<li><a href=\'\\([^>]\\+\\?\\.html\\)\' target=\"_blank\">\\([^<]\\+\\?\\)</a></li>", REG_ICASE);  //人民1网站2标题
	regcomp(&rrcom, "<li><a href=\'\\([^>]\\+\\?\\.html\\)\' target=_blank>\\([^<]\\+\\?\\)</a></li>", REG_ICASE);  //AL1网站2标题网站缺主机

	FILE* pTextFileHandler = fopen(pGblCurTextFile, "a+"); 	
	
	int m = 0; 
	int n = 0; 
	int y = 0; 
	int x = 0; 
	int z = 0; 
	int r = 0; 
	int i; 

	
	while(i < 50)
	{
		if(y != -1)
		{
			y = regular_match(&rycom, rygch, ytem, y, pTextFileHandler, 3); 
		}

		if(x != -1)
		{
			x = regular_match(&rxcom, rxgch, xtem, x, pTextFileHandler, 3); 
		}

		if(z != -1)
		{
			z = regular_match(&rzcom, rzgch, ztem, z, pTextFileHandler, 3); 
		}

		if(r != -1)
		{
			r = regular_match(&rrcom, rrgch, rtem, r, pTextFileHandler, 3); 
		}

		if(y == -1 && x == -1 && z == -1 && r == -1)
			break; 
		
		i++; 
	}
	
		
	FILE* pJpgFileHandler = fopen(pGblCurJpgFile, "a+"); 	
	i = 0; 

	while(i < 50)
	{

		if(m != -1)
		{
			m = regular_match(&recom, regch, strtem, m, pJpgFileHandler, 4); 
		}
			
		
		if(n != -1)
		{
			n = regular_match(&racom, ragch, atem, n, pJpgFileHandler, 4); 
		}

		if(m == -1 && n == -1)
			break; 
	}



	if(arg != NULL)
	{
		send_text_file(pReqNetData, pTextFileHandler); 
		send_text_file(pReqNetData, pJpgFileHandler); 
	}
	
	fclose(pTextFileHandler); 
	fclose(pJpgFileHandler); 
	

	regfree(&racom); 
	regfree(&recom); 
	regfree(&rycom); 
	regfree(&rxcom); 
	regfree(&rzcom); 
	regfree(&rrcom); 

	munmap(string, len); 
	munmap(atem, len); 
	munmap(ytem, len); 
	munmap(xtem, len); 
	munmap(ztem, len); 
	munmap(rtem, len); 
	close(fd); 

	if(pReqNetData != NULL)
		free(pReqNetData); 

	remove(u.szFilePath); 
	
	return 0; 
}


void* thread_time_update_func(void* arg)
{
	int datefd = -1; 
	while(TRUE)
	{
		time(&pGlobalTimeFile); 
		tm_finame = localtime(&pGlobalTimeFile); 
		memset(pGblCurTextFile, '\0', sizeof(pGblCurTextFile)); 
		memset(pGblCurJpgFile, '\0', sizeof(pGblCurJpgFile)); 
		memset(szGlbTodayTime, '\0', sizeof(szGlbTodayTime)); 

		sprintf(
				szGlbTodayTime, 
				"%d%d%d", 
				1900 + tm_finame->tm_year, 
				tm_finame->tm_mon + 1, 
				tm_finame->tm_mday
				); 

		sprintf(pGblCurTextFile, 
				"Title/Txt%d%d%d.txt", 
				1900 + tm_finame->tm_year, 
				tm_finame->tm_mon + 1,
			   	tm_finame->tm_mday
				); 

		sprintf(pGblCurJpgFile, 
				"Title/Jpg%d%d%d.txt", 
				1900 + tm_finame->tm_year, 
				tm_finame->tm_mon + 1, 
				tm_finame->tm_mday
				); 

		datefd = open(pGblCurTextFile, O_RDONLY); 
		if(datefd == -1)
		{
			destroy_hash_table(pGlobalHashTable);   //哈西表销毁
			update_all_news(NULL); 
		}else{
			close(datefd); 
			datefd = -1; 
			update_all_news(NULL); 
		}
		sleep(28800); 
	}
}
