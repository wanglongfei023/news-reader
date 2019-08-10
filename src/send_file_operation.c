/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/05/05 11:03:34
*	Description:
*
==========================================================================*/

#include<news_reader.h>

int send_text_file(net_data_t* pNetData, FILE* pFileHandler)
{
	if(pFileHandler == NULL)
	{
		LOG("warning: the text file handler is null so do nothing.");
		return 0;
	}
	unsigned int nFileLen; 
	unsigned char szBuff[4096]; 
	int nWriteNum, nReadNum, nSendNum; 
	nWriteNum = 0; 
	nReadNum = 0; 
	nSendNum = 0; 
	net_data_t netData = *pNetData; 
	memset(szBuff, 0, sizeof(szBuff)); 
	
	if(pFileHandler != NULL)
	{
		fseek(pFileHandler, 0, SEEK_END); 
		nFileLen = ftell(pFileHandler); 
		netData.nBuffLen = nFileLen; 
		LOG("send num:%d", nFileLen); 
		write(netData.clifd, &netData, sizeof(netData)); 
		nWriteNum = 0; 
		nReadNum = 0; 
		fseek(pFileHandler, 0, SEEK_SET); 
		while(TRUE)
		{
			nReadNum = fread(szBuff, sizeof(unsigned char), sizeof(szBuff), pFileHandler); 
			if(nReadNum > 0)
			{
				while(nReadNum > 0)
				{
					nWriteNum = write(netData.clifd, szBuff + nWriteNum, nReadNum - nWriteNum); 
					nReadNum = nReadNum - nWriteNum; 
					nSendNum = nSendNum + nWriteNum; 
				}
				memset(szBuff, 0, sizeof(szBuff)); 
				nReadNum = 0; 
				nWriteNum = 0; 
			}

			if(nSendNum == nFileLen)
				break; 
		}
		LOG("debug: send text file and file length is %d.", nFileLen);
	}else{
		write(netData.clifd, &netData, sizeof(netData)); 
	}
	return 1; 
}


int send_binary_file(net_data_t* pNetData, FILE* pFileHandler)
{
	if(pFileHandler == NULL)
	{
		LOG("warning: the text file handler is null so do nothing.");
		return 0;
	}

	unsigned char buf[4096]; 
	int nFileLen, nSendNum, nReadNum, nDataLen; 
	int nWriteNum; 
	net_data_t donedate; 
	memset(donedate.buf, '\0', sizeof(donedate.buf)); 
	memset(donedate.url, '\0', sizeof(donedate.url)); 
	donedate.nPackType = 0; 
	donedate.nBuffLen = 0; 
	donedate.clifd = -1; 
	
	bzero(buf, sizeof(buf)); 
	net_data_t netData = *pNetData; 

	fseek(pFileHandler, 0, SEEK_END); 
	nFileLen = ftell(pFileHandler); 
	fseek(pFileHandler, 0, SEEK_SET); 
	netData.nBuffLen = nFileLen; 
	
	nWriteNum = 0; 
	nDataLen = 0; 
	
	while(nDataLen < sizeof(netData))
	{
		nWriteNum = write(netData.clifd, &netData, sizeof(netData)); 
		nDataLen = nWriteNum + nDataLen; 
	}
	
	nWriteNum = 0; 
	nDataLen = 0; 

	while((nReadNum = fread(buf, sizeof(unsigned char), sizeof(buf), pFileHandler)) != 0)
	{
		while(nWriteNum < nReadNum)
		{
				nWriteNum = write(netData.clifd, buf +nDataLen, nReadNum-nDataLen); 
				nDataLen = nWriteNum+nDataLen; 
				nSendNum = nDataLen+nSendNum; 
		}

		nWriteNum = 0; 
		nDataLen = 0; 
		bzero(buf, sizeof(buf)); 
		nReadNum = 0; 
	}
	LOG("debug: send binary file and file length is %d", nFileLen);
	return 1; 

}
