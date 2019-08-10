/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/05/08 14:03:56
*	Description:
*
==========================================================================*/

#include<news_reader.h>

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

		sprintf(
				pGblCurTextFile, 
				"Title/Txt%d%d%d.txt", 
				1900 + tm_finame->tm_year, 
				tm_finame->tm_mon + 1,
			   	tm_finame->tm_mday
				); 

		sprintf(
				pGblCurJpgFile, 
				"Title/Jpg%d%d%d.txt", 
				1900 + tm_finame->tm_year, 
				tm_finame->tm_mon + 1, 
				tm_finame->tm_mday
				); 

		datefd = open(pGblCurTextFile, O_RDONLY); 
		printf("\n\n\n\n\n%s\n\n\n\n", pGblCurTextFile);
		if(datefd == -1)
		{
			destroy_hash_table(pGlobalHashTable);   //哈西表销毁
			update_all_news(NULL); 
		}else{
			close(datefd); 
			datefd = -1; 
			update_all_news(NULL); 
		}

		char szCmd[128] = {0};
		sprintf(szCmd, "./script/add_blacklist.sh %s", pGblCurTextFile);
		//printf("%s\n", szCmd);
		system(szCmd);
		sleep(28800); 
	}
}
