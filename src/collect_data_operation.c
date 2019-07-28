/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/05/03 11:09:58
*	Description:
*
==========================================================================*/

#include<news_reader.h>

char* split_string(char* pFileMapStr, char* buf, int* nlen)
{
	char* idex; 
	char* jdex; 
	char qq[20]; 
	strcpy(qq, buf); 
	idex = strstr(pFileMapStr, buf); 
	jdex = strstr((pFileMapStr + strlen(qq)), buf); 
	bzero(buf, 300); 
	strncpy(buf, idex+strlen(qq), jdex - idex - strlen(qq)); 
	*nlen = jdex - pFileMapStr + strlen(qq); 

	return jdex+strlen(qq); 
}

int collect_string_into_hash(hash_t** pHashTable, int fd, int num)
{
	if(pHashTable == NULL || fd == -1)
		return -1; 

	int len = lseek(fd, 0, SEEK_END); 
	char* pFileMapStr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0); 
	char url[256]; 
	char szFilePath[256]; 
	char buf[256]; 
	char* pStr = pFileMapStr; 	
	time_t tTime = 0; 


	int i = 1; 
	int n = 0; 

	int nlen = 0; 
	int m = 0; 
	
	while((nlen + 1) < len)
	{
		if(num == 1)
		{
			for(i = 1; i <= 2; i++)
			{
				sprintf(buf, "<%d>", i); 
				pStr = split_string(pStr, buf, &m); 
				nlen = nlen + m; 
				m = 0; 
				if(i == 1)
					strcpy(url, buf); 
				else
					strcpy(szFilePath, buf); 
				bzero(buf, sizeof(buf)); 
			}
		}

		if(num == 2)
		{
			for(i = 1; i <= 3; i++)
			{
				sprintf(buf, "<%d>", i); 
				pStr = split_string(pStr, buf, &m); 
				nlen = nlen + m; 
				m = 0; 
				if(i == 1)
					strcpy(url, buf); 
				else if(i == 3)
					strcpy(szFilePath, buf); 
				bzero(buf, sizeof(buf)); 
			}

		}

		strcpy(buf, "<t>"); 
		pStr = split_string(pStr, buf, &m); 
		nlen = nlen + m; 

		m = 0; 
		tTime = strtol(buf, NULL, 10); 
		hash_push(pGlobalHashTable, url, szFilePath, tTime); 

		bzero(buf, sizeof(buf)); 
		bzero(url, sizeof(url)); 
		bzero(szFilePath, sizeof(szFilePath)); 
		tTime = 0; 
		n++; 
	}
	
	munmap(pFileMapStr, len); 
	return n; 
}


int regular_match(regex_t* recom, regmatch_t* regch, char* string, int n, FILE* pFileHandler, int Renum)
{
	char szFilePath[8192]; 
	char url[1024]; 
	char ming[1024]; 
	int t = regch[0].rm_eo; 	
	int k; 
	int blase = 1; 
	int i = 0; 

	regexec(recom, string + n, Renum, regch, 0); 

	k = regch[0].rm_eo + n; 

	if(t == regch[0].rm_eo)
	{
		return -1; 
	}

	while(i < Renum)
	{	
		if(i == 0 && Renum > 1)
			i = 1; 

		snprintf(szFilePath, regch[i].rm_eo - regch[i].rm_so + 4, "<%d>%s", i, string + n + regch[i].rm_so); 
		if(blase == 1 && Renum > 1)
		{
			if(Renum == 3)
			{
				snprintf(url, regch[1].rm_eo-regch[1].rm_so + 1, "%s", string + n + regch[1].rm_so); 
				snprintf(ming, regch[2].rm_eo-regch[2].rm_so + 1, "%s", string + n + regch[2].rm_so); 
				blase = hash_push(pGlobalHashTable, url, ming, 0); 
				if(blase == 0)
				{
					return k; 
				}
			}

			if(Renum == 4)
			{
				snprintf(url, regch[1].rm_eo-regch[1].rm_so + 1, "%s", string + n + regch[1].rm_so); 
				snprintf(ming, regch[3].rm_eo-regch[3].rm_so + 1, "%s", string + n + regch[3].rm_so); 
				blase = hash_push(pGlobalHashTable, url, ming, 0); 
				if(blase == 0)
				{
					return k; 
				}
			}

			blase = 0; 

		}

		fprintf(pFileHandler, szFilePath, 0); 		
		bzero(szFilePath, sizeof(szFilePath)); 
		snprintf(szFilePath, 4, "<%d>", i); 
		fprintf(pFileHandler, szFilePath, 0); 		
		bzero(szFilePath, sizeof(szFilePath)); 
		++i; 
	}

	bzero(szFilePath, sizeof(szFilePath)); 
	sprintf(szFilePath, "<t>%s<t>\n", szGlbTodayTime); 
	fprintf(pFileHandler, szFilePath, 0); 

	return k; 

}
