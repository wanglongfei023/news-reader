/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/07/26 15:09:58
*	Description: 封装用户验证码操作和报警操作函数接口
*
==========================================================================*/

#include <news_reader.h>
char* pConfig[]={
				   "-S from=183593190@qq.com\
			   	 	-S smtp=smtps://smtp.qq.com\
			     	-S smtp-auth-user=183593190@qq.com\
			   		-S smtp-auth-password=hbgkpcyzlttkbiij\
			   		-S smtp-auth=login",

				   "-S from=395592722@qq.com\
			   	 	-S smtp=smtps://smtp.qq.com\
			     	-S smtp-auth-user=395592722@qq.com\
			   		-S smtp-auth-password=xdnhsboithogbjbc\
			   		-S smtp-auth=login"
				 };	
				   

void send_mail(const char* pMailAddress, const char* pTheme, const char* pInfo)
{
	static int nConfigIndex = 0;

	char szCmd[512] = {0};
	sprintf(
			szCmd, 
			"echo '%s' | mail -s \"%s\" %s %s>> ./log/mail.log", 
			pInfo, 
			pTheme, 
			pConfig[nConfigIndex],
			pMailAddress
		   );
	printf("%s\n", szCmd);
	system(szCmd);

	nConfigIndex++;
	nConfigIndex %= sizeof(pConfig) / sizeof(char*);
}

void send_varify_code(const char* pMailAddress, const char* pCode)
{
	
	char szInfo[256] = {0};
	
	//获取当前时间来改变信息内容避免被SMTPP服务器列入垃圾名单
	time_t tmpTime = time(NULL); 
	struct tm* pFormatTime = localtime(&tmpTime); 

	sprintf(
			szInfo, 
			"Your verification code of registering is %s.\
			\nIf it is not your own operation, please ignore this information.\
			\n\n            %04d-%02d-%02d %02d:%02d:%02d",
			pCode,
			pFormatTime->tm_year + 1900,  
			pFormatTime->tm_mon + 1,  
			pFormatTime->tm_mday,  
			pFormatTime->tm_hour,
			pFormatTime->tm_min,
			pFormatTime->tm_sec
		   );


	//避免相同的标题被SMTP邮件服务器列入垃圾邮件列表
	const char* szThemes[] = { 
								"THE VERIFICATION CODE", "VERIFICATION CODE ENSURE",
						    	"YOUR VERIFICATION CODE", "ENSURE YOUR VERIFICATION",
						    	"NEWS-READER VARIFICATION", "LOOK FORWARD YOUR COMING",
								"JOIN US -----NEWS-READER", "REGISTER'S VARIFCATION"
							  };
	srand(time(0));
	int nIndex = rand() % (sizeof(szThemes) / sizeof(char*));
	send_mail(pMailAddress, szThemes[nIndex],  szInfo);
}

void mail_notify(const char* pInfo)
{
	const char* pMailAddress = "395592722@qq.com";

	//避免相同的标题被SMTP邮件服务器列入垃圾邮件列表
	const char* szThemes[] = { 
								"NOTICE FROM NEWSREADER", "NEWSREADER INFORMATION",
						    	"YOUR SERVER INFORMATION", "FROM NEWS-READER NOTICE",
						    	"----MAYBE BUG IS COMING", "BE CARE OF NEWS-READER"
						 	  };
	srand(time(0));
	int nIndex = rand() % (sizeof(szThemes) / sizeof(char*));

	send_mail(pMailAddress, szThemes[nIndex], pInfo);
}
/*
int main()
{
	const char* pMail = "395592722@qq.com";
	send_varify_code(pMail, "4321");
	mail_notify("test notify..");
}
*/
