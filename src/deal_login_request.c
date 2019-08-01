/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/07/30 21:50:26
*	Description:
*
==========================================================================*/

#include <news_reader.h>

void* deal_login_request(void* arg)
{
	//构造请求和回复协议包
	log_in_req* pLoginReq = (log_in_req*)arg;
	log_in_res* pLoginRes = (log_in_res*)malloc(sizeof(log_in_res));
	pLoginRes->nType = LOG_IN_RES;
	pLoginRes->nResult = _user_not_exist;
	bzero(&pLoginRes->uInfo, sizeof(pLoginRes->uInfo));

	//获取数据库句柄
	MYSQL* pSQLHandler0 = connect_database("localhost", "root", "", "news_reader");
	MYSQL* pSQLHandler1 = connect_database("localhost", "root", "", "news_reader");

	//检查获取用户信息
	if(get_user_info(pSQLHandler0, pLoginReq->szMail, &pLoginRes->uInfo) == _user_not_exist)
	{
		LOG(
			"debug: user log in failed and user is not exist. [user mail: %s][return code: %d]", 
			pLoginReq->szMail,
			_user_not_exist
		   );
		pLoginRes->nResult = _user_not_exist;
	}else if(check_user_info(pSQLHandler1, pLoginReq->szMail, pLoginReq->szPasswd) == _passwd_wrong){
		LOG(
			"debug: user log in failed and user's password is wrong. [user mail: %s][return code: %d]", 
			pLoginReq->szMail,
			_passwd_wrong
		   );
		pLoginRes->nResult = _passwd_wrong;
	}else{
		LOG(
			"debug: user log in success. [user mail: %s][return code: %d]", 
			pLoginReq->szMail,
			_passwd_right
			);
		pLoginRes->nResult = _passwd_right;
	}
		
	write(pLoginReq->nClientFd, pLoginRes, sizeof(log_in_res));

	//内存释放
	mysql_close(pSQLHandler0);
	mysql_close(pSQLHandler1);
	free(pLoginRes);
	//pLoginRes = NULL;

	return NULL;

}
