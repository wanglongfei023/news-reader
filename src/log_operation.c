/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/04/24 15:03:25
*	Description: 实现无日志等级的简单日志打印，包含打印处线程id、文件名、
*				 函数、行号等信息
*
==========================================================================*/

#include <news_reader.h>

int init_log(const char* pLogFileHandlerPath)
{
	time_t _time; 
	time(&_time); 
	char szTime[64] = {0}; 
	char szFile[128] = {0}; 
	strftime(szTime,  sizeof(szTime),  "%Y%m%d%_H%M%S",  localtime(&_time) ); 
	sprintf(szFile,  "%s.%s",  pLogFileHandlerPath,  szTime); 
	pLogFileHandler = fopen(szFile,  "a+"); 
	if(pLogFileHandler == NULL)
	{
		printf("ERROR: please check log path `%s`",  szFile); 
		return 0; 
	}
	return 1; 
}

int write_log(const char* pFile,  const char* pFunc,  int nLine,  const char *pFormat,  ...) 
{
	int result = 0; 
	if(pLogFileHandler == NULL)
		return result; 

	va_list arg; 
	va_start(arg,  pFormat); 
	time_t tmpTime = time(NULL); 
	struct tm* pFormatTime = localtime(&tmpTime); 

	char szBuff[128] = {0}; 
	vsprintf(szBuff,  pFormat,  arg); 
	va_end(arg); 
	char szLogInfo[256] = {0}; 

	sprintf(
			szLogInfo,  
			"%04d-%02d-%02d %02d:%02d:%02d [tid:0x%x] [file:%s] [func:%s] [line:%d]\
			\ninformation: %s\n\n",  
			pFormatTime->tm_year + 1900,  
			pFormatTime->tm_mon + 1,  
			pFormatTime->tm_mday,  
			pFormatTime->tm_hour,  
			pFormatTime->tm_min,  
			pFormatTime->tm_sec, 
			(unsigned int)pthread_self(), 
			pFile,  
			pFunc,  
			nLine, 
			szBuff
		   ); 

	printf("%s",  szLogInfo); 
	fprintf(pLogFileHandler,  "%s",  szLogInfo); 

	fflush(pLogFileHandler); 
	return 1; 
}

void drop_log()
{
	if(pLogFileHandler != NULL)
		fclose(pLogFileHandler); 

}

