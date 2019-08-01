/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/05/02 18:03:20
*	Description: 处理用户的数据请求，包含最新新闻标题，文字新闻、图片新闻
*				 视频新闻等
*
==========================================================================*/

#include<news_reader.h>

void* push_client_current_news(void* arg)
{
	net_data_t* pNetData = (net_data_t*)arg; 

	pNetData->nPackType = _CURRENT_NEWS_REQUEST; 
	strcpy(pNetData->url, "人民网\0"); 
	LOG("debug: deal request of updating for client.");
	
	FILE* pTextFileHandler = NULL; 
    FILE* pPictureFileHandler = NULL; 
    FILE* pVideoFileHandler = NULL; 

	pTextFileHandler = fopen(pGblCurTextFile, "rb"); 
	pPictureFileHandler = fopen(pGblCurJpgFile, "rb"); 
	pVideoFileHandler = fopen(pGblCurVedioFile, "rb"); 

    if(pTextFileHandler == NULL || pPictureFileHandler == NULL || pVideoFileHandler == NULL)
    {
    	pNetData->nPackType = FAILED; 
        send_text_file(pNetData, NULL); 
        if(pTextFileHandler != NULL)
            fclose(pTextFileHandler); 
        if(pPictureFileHandler != NULL)
            fclose(pPictureFileHandler); 
        if(pVideoFileHandler != NULL)
            fclose(pVideoFileHandler); 

        return NULL; 

     }

	net_data_t newdate; 

	send_text_file(pNetData, pTextFileHandler); 
	fclose(pTextFileHandler); 
	read(pNetData->clifd, &newdate, sizeof(net_data_t)); 
	send_text_file(pNetData, pPictureFileHandler); 
	fclose(pPictureFileHandler); 
	read(pNetData->clifd, &newdate, sizeof(net_data_t)); 
	send_text_file(pNetData, pVideoFileHandler); 
	fclose(pVideoFileHandler); 
	read(pNetData->clifd, &newdate, sizeof(net_data_t)); 

	return NULL; 

}


void* deal_picture_request(void* arg)
{
	net_data_t responseNetData; 
	net_data_t requestNetData = *(net_data_t*)arg; 
	url_t u; 
	int nWriteNum = 0; 
	int nReadNum = 0; 

	requestNetData.nPackType = _PICTURE_NEWS_REQUEST; 
	responseNetData.nPackType = 0; 
	strcpy(responseNetData.buf, "没有数据"); 
	responseNetData.nBuffLen = strlen(responseNetData.buf); 
	
	LOG("debug: update picture for client.");

	strcpy(u.url, requestNetData.url); 
	sprintf(u.szFilePath, "Jpg/%s", requestNetData.buf); 
	strcat(u.szFilePath, "\0"); 

	FILE* pFileHandler = fopen(u.szFilePath, "rb"); 
	if(pFileHandler != NULL)  // 本地文件该图片没有缓存
	{
		send_binary_file(&requestNetData, pFileHandler); 
		fclose(pFileHandler); 
		return NULL; 
		
	}

	if(analyze_url(&u, _UPDATE_ALL_NEWS) == -1)   //根据url信息爬取
	{
		nWriteNum = 0; 
		nReadNum = 0; 
		while(nReadNum < sizeof(responseNetData))
		{
			nWriteNum = write(requestNetData.clifd, &responseNetData, sizeof(responseNetData)); 
			nReadNum = nWriteNum + nReadNum; 
		}
		return NULL; 
	}

	int webfd = create_connect_socket(); 

	if(connect_web_server(&u, webfd) == -1)
	{
		nWriteNum = 0; 
		nReadNum = 0; 
		while(nReadNum < sizeof(responseNetData))
		{
			nWriteNum = write(requestNetData.clifd, &responseNetData, sizeof(responseNetData)); 
			nReadNum = nWriteNum+nReadNum; 
		}
		return NULL; 
	}

	char head[4096]; 
	create_request_head(head, &u); 
	send_request_head(head, webfd, NULL); 

	if(get_web_response(&u, webfd, NULL) == -1)
	{
		nWriteNum = 0; 
		nReadNum = 0; 
		while(nReadNum < sizeof(responseNetData))
		{
			nWriteNum = write(requestNetData.clifd, &responseNetData, sizeof(responseNetData)); 
			nReadNum = nWriteNum+nReadNum; 
		}
		return NULL; 
	}

	pFileHandler = fopen(u.szFilePath, "rb"); 

	if(pFileHandler == NULL)
	{
		nWriteNum = 0; 
		nReadNum = 0; 
		while(nReadNum < sizeof(responseNetData))
		{
			nWriteNum = write(requestNetData.clifd, &responseNetData, sizeof(responseNetData)); 
			nReadNum = nWriteNum+nReadNum; 
		}
		return NULL; 
	}	
	send_binary_file(&requestNetData, pFileHandler); 
	fclose(pFileHandler); 
	
	return NULL; 

}


void* deal_video_request(void* arg)
{
	net_data_t* pRequestData = (net_data_t*)arg; 
	char szFilePath[256]; 
	sprintf(szFilePath, "Video/%s", pRequestData->buf); 

	pRequestData->nPackType = _VIDEO_NEWS_REQUEST; 
	strcpy(pRequestData->buf, "更新视频信息\0"); 
	strcpy(pRequestData->url, "NULL\0"); 

	LOG("debug: deal update video request.");
	FILE* pFileHandler = fopen(szFilePath, "rb"); 
	
	if(pFileHandler == NULL)
	{
		return NULL; 
	}
	send_binary_file(pRequestData, pFileHandler); 
	fclose(pFileHandler); 

	return NULL; 
}


