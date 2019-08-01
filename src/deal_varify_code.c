/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/07/30 15:42:50
*	Description:
*
==========================================================================*/
#include <news_reader.h>

void* deal_varify_code_request(void* arg)
{
	//构造请求和回复包
	mail_varify_req* pVarifyReq = (mail_varify_req*)arg;
	mail_varify_res* pVarifyRes = (mail_varify_res*)malloc(sizeof(mail_varify_res));
	pVarifyRes->nType = MAIL_VARIFY_RES;

	//生成四位随机验证码
	srand(time(0));
	int nCode = rand() % 10000;
	char szVarifyCode[5] = {0};
	sprintf(szVarifyCode, "%04d", nCode);
	strcpy(pVarifyRes->szCode, szVarifyCode);

	LOG("debug: recive varify code request and send mail code is `%s`.", szVarifyCode);

	//向邮箱发送验证码
	send_varify_code(pVarifyReq->szMail ,szVarifyCode);

	//回复给客户端
	write(pVarifyReq->nClientFd, pVarifyRes, sizeof(mail_varify_res));

	//内存释放
	free(pVarifyRes);
	pVarifyRes = NULL;

	return NULL;
}
