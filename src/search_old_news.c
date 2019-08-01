/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/07/31 20:42:34
*	Description:
*
==========================================================================*/

#include <news_reader.h>

void* search_old_news(void* arg)
{

	net_data_t* pReqNetData = (net_data_t*)arg; 
	net_data_t reqNetData = *pReqNetData; 
	net_data_t resNetData; 
	FILE* pFileHandler = NULL; 
	char szFileName[128]; 
	char szReqPath[128]; 
	strcpy(szReqPath, reqNetData.url); 
	memset(szFileName, '\0', sizeof(szFileName)); 

	sprintf(szFileName, "Title/Txt%s.txt", szReqPath); 
	reqNetData.nPackType = _SEARCH_OLD_NEWS_REQUEST; 

	pFileHandler = fopen(szFileName, "rb"); 
	
	if(pFileHandler == NULL)
	{
		strcpy(reqNetData.buf, "FULL"); 
		reqNetData.nBuffLen = strlen(reqNetData.buf); 
		strcpy(reqNetData.url, "FULL"); 
		reqNetData.nPackType = FAILED; 
		send_text_file(&reqNetData, NULL); 
	}else{
		send_text_file(&reqNetData, pFileHandler); 
		fclose(pFileHandler); 
		pFileHandler = NULL; 
		read(pReqNetData->clifd, &resNetData, sizeof(net_data_t)); 
	}
	
	memset(szFileName, '\0', sizeof(szFileName)); 	
	sprintf(szFileName, "Title/Jpg%s.txt", szReqPath); 
	reqNetData.nPackType = _SEARCH_OLD_NEWS_REQUEST; 
	pFileHandler = fopen(szFileName, "rb"); 
	
	if(pFileHandler == NULL)
	{
		strcpy(reqNetData.buf, "FULL"); 
		reqNetData.nBuffLen = strlen(reqNetData.buf); 
		strcpy(reqNetData.url, "FULL"); 
		reqNetData.nPackType = 0; 
		send_text_file(&reqNetData, NULL); 
	}else{
		send_text_file(&reqNetData, pFileHandler); 
		fclose(pFileHandler); 
		pFileHandler = NULL; 
		read(pReqNetData->clifd, &resNetData, sizeof(net_data_t)); 
	}

	memset(szFileName, '\0', sizeof(szFileName)); 	
	sprintf(szFileName, "Title/Video%d.txt", 190011); 
	reqNetData.nPackType = _SEARCH_OLD_NEWS_REQUEST; 

	pFileHandler = fopen(szFileName, "rb"); 
	if(pFileHandler == NULL)
	{
		strcpy(reqNetData.buf, "FULL"); 
		reqNetData.nBuffLen = strlen(reqNetData.buf); 
		strcpy(reqNetData.url, "FULL"); 
		reqNetData.nPackType = 0; 
		send_text_file(&reqNetData, NULL); 
	}else{
		send_text_file(&reqNetData, pFileHandler); 
		fclose(pFileHandler); 
		pFileHandler = NULL; 
		read(pReqNetData->clifd, &resNetData, sizeof(net_data_t)); 
	}

	return (void*)TRUE; 

}
