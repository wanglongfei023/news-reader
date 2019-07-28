/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/04/28 19:03:25
*	Description:
*
==========================================================================*/

#include <news_reader.h>

hash_t** hash_init(int nLength)
{
	int i = 0; 
	hash_t** phash = (hash_t**)malloc(sizeof(hash_t*)*nLength); 
	if(phash == NULL)
	{
		LOG("warning: fail to init hash table.");
		return NULL; 
	}
	
	for(i = 0; i < nLength; i++)
	{
		phash[i] = NULL; 
	}

	return  phash; 

}

int hash_push(hash_t** pHash, char* pUrlVal, char* pNewsName, time_t saveTime)
{
	int hash = 0; 
	int x = 0; 
	int node = 0; 

	char* url = NULL; 
	char* strstr = NULL; 

	hash = 0; 
	x = 0; 
	node = 0; 
	url = pUrlVal; 
	strstr = pUrlVal; 

	while(*url)
	{
		hash = (hash << 4) + *url; 

		if((x = hash & 0xf0000000) != 0)
		{
			hash ^= (x >> 24); 
			hash &= ~x; 
		}

		url++; 
	}

	node = (hash & 0x7fffffff) % 1000; 

	hash_t* clihash = hash_search(pHash, strstr); 

	if(clihash != NULL)
	{
		return 0; 
	}
	clihash = (hash_t*)malloc(sizeof(hash_t)); 

	strcpy(clihash->url, pUrlVal); 
	strcpy(clihash->szFilePath, pNewsName); 
	if(saveTime == 0)
		clihash->time_num = atoi(szGlbTodayTime);  
	else
		clihash->time_num = saveTime; 
	clihash->pNext = NULL; 
	clihash->hash_len = HASH_LEN; 

	hash_t** tthash = &pHash[node]; 

	while((*tthash) != NULL)
	{
		tthash = &((*tthash)->pNext);   
	}
  
	*tthash = clihash; 

	return 1; 

}

hash_t* hash_search(hash_t** pHashTable, char* pString)
{
	int hash = 0; 
	int x = 0; 
	int node; 
	
	char* url = pString; 

	while(*url)
	{
		hash = (hash << 4) + *url; 

		if((x = hash & 0xf0000000) != 0)
		{
			hash ^= (x >> 24); 
			hash &= ~x; 
		}

		url++; 
	}

	node = (hash & 0x7fffffff) % 1000; 

	hash_t* thash = NULL; 
	
	if(pHashTable[node] != NULL)
	{
		thash = pHashTable[node]; 
		while(thash != NULL)
		{
			x = strcmp(thash->url, pString); 
			if(x == 0)
			{
				return thash; 
			}
			thash = thash->pNext; 
		}

		return NULL; 
	}

	return NULL; 

}

int destroy_hash_table(hash_t** phash)
{
	if(phash == NULL)
		return 0; 

	int i = 0; 
	int j = 0; 
	hash_t* xhash = NULL; 
	hash_del* thash = (hash_del*)malloc(sizeof(hash_del)); 
	
	for(i = 0; i < 5; i++)
	{
		thash->arr[i] = NULL; 
	}

	thash->num = 0; 

	for( i= 0; i < HASH_LEN; i++)
	{
		if(phash[i] != NULL)
		{
			xhash = phash[i]; 
			while(TRUE)
			{
				if(thash->num == 5 || xhash == NULL)
				{
					j = 0; 
					while(j < 5)
					{
						if(thash->arr[j] != NULL)
						{
							free(thash->arr[j]); 
							thash->arr[j] = NULL; 
						}
						else	
							break; 
						j++; 
					}

					thash->num = 0; 
				}

				if(xhash == NULL)
					break; 

				thash->arr[thash->num] = xhash; 
				xhash = xhash->pNext; 
				thash->num++; 
			}

		}

	}

	free(thash); 

	for(i = 0; i < HASH_LEN; i++)
	{
		phash[i] = NULL; 
	}	

	return 1; 
}
