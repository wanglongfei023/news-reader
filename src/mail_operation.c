/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/07/26 15:09:58
*	Description: 封装用户验证码操作和报警操作函数接口
*
==========================================================================*/

#include <news_reader.h>

void send_mail(const char* pMailAddress, const char* pTheme, const char* pInfo)
{
	char szCmd[512] = {0};
	sprintf(szCmd, "echo '%s' | mail -s \"%s\" %s >> mail.log", pInfo, pTheme, pMailAddress);
	//printf("%s\n", szCmd);
	system(szCmd);
}

void send_varify_code(const char* pMailAddress, const char* pCode)
{
	char szInfo[128] = {0};
	sprintf(
			szInfo, 
			"from news-reader:  Your verification code is %s.\
			 \nif it is not your own operation, please ignore this information. ",
			 pCode
		   );
	send_mail(pMailAddress,"The Verification Code",  szInfo);
}

void mail_notify(const char* pInfo)
{
	const char* pMailAddress = "395592722@qq.com";
	const char* pTheme = "news-reader's notify";

	send_mail(pMailAddress, pTheme, pInfo);
}

/*
int main()
{
	const char* pMail = "395592722@qq.com";
	send_varify_code(pMail, "4321");
	mail_notify("test notify..");
}
*/
