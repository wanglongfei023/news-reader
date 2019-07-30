/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/07/30 16:38:00
*	Description:
*
==========================================================================*/

#include <news_reader.h>

void* deal_register_request(void* arg)
{
	//构造请求回复包
	register_req* pRegisterReq = (register_req*)arg;
	register_res* pRegisterRes = (register_res*)malloc(sizeof(register_res));
	pRegisterRes->nType = REGISTER_RES;

	//查询和插入用户信息
	MYSQL* pSQLHandler = connect_database("localhost", "root", "", "news_reader");
	//用户已注册
	if(check_user_info(pSQLHandler, pRegisterReq->szMail, pRegisterReq->szPasswd) != _user_not_exist)
	{
		pRegisterRes->nResult = _user_exist;
		write(pRegisterReq->nClientFd, pRegisterRes, sizeof(register_res));
	}else{
		insert_user(pSQLHandler, pRegisterReq->szMail, pRegisterReq->szName, pRegisterReq->szPasswd);
		pRegisterRes->nResult = _register_success;
		write(pRegisterReq->nClientFd, pRegisterRes, sizeof(register_res));
	}

	return NULL;
}
