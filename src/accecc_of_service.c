/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/04/26 12:09:58
*	Description:
*
==========================================================================*/

#include<news_reader.h>

int handle_request(pool_t* pPool, net_data_t* pNetData)
{
	net_data_t clientData = *pNetData; 
	switch(clientData.nPackType)
	{
		
		case _UPDATE_ALL_NEWS:
			thread_task_add(pPool, update_all_news, (void*)&clientData); 
			break; 

		case _CURRENT_NEWS_REQUEST:
			thread_task_add(pPool, push_client_current_news, (void*)&clientData); 
			break; 

		case _SEARCH_ONE_NEWS_REQUEST:
			thread_task_add(pPool, search_news_for_client, (void*)&clientData); 
			break; 

		case _SEARCH_OLD_NEWS_REQUEST:
			thread_task_add(pPool, search_old_news, (void*)&clientData); 
			break; 

		case _VIDEO_NEWS_REQUEST:
			thread_task_add(pPool, deal_video_request, (void*)&clientData); 
			break; 

		case _PICTURE_NEWS_REQUEST:
			thread_task_add(pPool, deal_picture_request, (void*)&clientData); 
			break; 

		default:
			break; 
	}

	return 1; 
}
