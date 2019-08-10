/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/04/26 12:09:58
*	Description:
*
==========================================================================*/

#include<news_reader.h>

int handle_request(int nClientFd, char* pRecvBuff)
{
	PackType* pPackType = (PackType*)pRecvBuff; 
	switch(*pPackType)
	{
		case MAIL_VARIFY_REQ:
			{
				mail_varify_req* pVarifyReq = (mail_varify_req*)pRecvBuff;
				pVarifyReq->nClientFd = nClientFd;
				thread_task_add(pGlobalPool, deal_varify_code_request, (void*)pVarifyReq); 
				break;
			}		

		case REGISTER_REQ:
			{
				register_req* pRegisterReq = (register_req*)pRecvBuff;
				pRegisterReq->nClientFd = nClientFd;
				thread_task_add(pGlobalPool, deal_register_request, (void*)pRegisterReq); 
				break;
			}

		case LOG_IN_REQ:
			{
				log_in_req* pLoginReq = (log_in_req*)pRecvBuff;
				pLoginReq->nClientFd = nClientFd;
				thread_task_add(pGlobalPool, deal_login_request, (void*)pLoginReq);
				break;
			}

		case _UPDATE_ALL_NEWS:
			{
				net_data_t* pRecvNetData = (net_data_t*)pRecvBuff;
				pRecvNetData->clifd = nClientFd;
				thread_task_add(pGlobalPool, update_all_news, (void*)pRecvNetData); 
				break; 
			}

		case _CURRENT_NEWS_REQUEST:
			{
				net_data_t* pRecvNetData = (net_data_t*)pRecvBuff;
				pRecvNetData->clifd = nClientFd;
				thread_task_add(pGlobalPool, push_client_current_news, (void*)pRecvNetData); 
				break; 
			}

		case _SEARCH_ONE_NEWS_REQUEST:
			{
				net_data_t* pRecvNetData = (net_data_t*)pRecvBuff;
				pRecvNetData->clifd = nClientFd;
				thread_task_add(pGlobalPool, search_news_for_client, (void*)pRecvNetData); 
				break; 
			}

		case _SEARCH_OLD_NEWS_REQUEST:
			{
				net_data_t* pRecvNetData = (net_data_t*)pRecvBuff;
				pRecvNetData->clifd = nClientFd;
				thread_task_add(pGlobalPool, search_old_news, (void*)pRecvNetData); 
				break; 
			}

		case _VIDEO_NEWS_REQUEST:
			{
				net_data_t* pRecvNetData = (net_data_t*)pRecvBuff;
				pRecvNetData->clifd = nClientFd;
				thread_task_add(pGlobalPool, deal_video_request, (void*)pRecvNetData); 
				break; 
			}

		case _PICTURE_NEWS_REQUEST:
			{
				net_data_t* pRecvNetData = (net_data_t*)pRecvBuff;
				pRecvNetData->clifd = nClientFd;
				thread_task_add(pGlobalPool, deal_picture_request, (void*)pRecvNetData); 
				break; 
			}

		default:
			return 0;
	}
	return 1; 
}
