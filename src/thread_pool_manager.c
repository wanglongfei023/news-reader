/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/05/06 12:03:18
*	Description:
*
==========================================================================*/

#include<news_reader.h>

void* thread_manager_func(void* arg)
{
	pool_t* pool = (pool_t*)arg; 
	
	int cur; 
	int alive; 
	int busy; 
	int i = 0; 
	int add = 0; 
	LOG("debug: thread manager begin to work.");

	while(pool->shut_down)
	{
		pthread_mutex_lock(&pool->lock); 
		cur = pool->queue_cur; 
		alive = pool->thread_alive; 
		busy = pool->thread_busy; 
		pthread_mutex_unlock(&pool->lock); 
		

		//判断扩容条件
		if((cur >= alive-busy || (float)busy/alive*100 >= (float)80 || alive < 10) &&
			  						    ((alive+pool->thread_min) < pool->thread_max))
		{
			for(i=0, add=0; i < pool->thread_max&&add<pool->thread_min; i++)
			{
				if(pool->tids[i] == 0 || !if_thread_alive(pool->tids[i]))
				{
					pthread_create(&pool->tids[i], &pool->arr, thread_worker_func, (void*)pool); 
					pthread_mutex_lock(&pool->lock); 
					++pool->thread_alive; 
					pthread_mutex_unlock(&pool->lock); 
					add++; 
				}
			}

			add = 0; 

		}

		//判断缩减条件
		if(busy * 2 <= (alive - busy) && ((alive - busy - pool->thread_min) >= pool->thread_min))
		{
			pthread_mutex_lock(&pool->lock); 
			pool->killno = pool->thread_min; 
			pthread_mutex_unlock(&pool->lock); 
			
			for(i=0; i<pool->thread_min; i++)
			{
				pthread_cond_signal(&pool->empty); 
			}
		}

		LOG("debug: Manager thread tid:0x%x\tpthread_pool info:\
			\nALIVE:%d\tBUSY:%d\t LIE:%d\t B/A:%.2f%%\t A/ALL:%.2f%%\n", 
			(unsigned int)pthread_self(), 
			alive, 
			busy, 
			alive - busy, 
			(float)busy / alive * 100, 
			(float)alive / pool->thread_max * 100
			); 
		sleep(TIMEOUT); 	
	}

	pthread_exit(NULL); 
}

int if_thread_alive(pthread_t tid)
{
	int m; 
	if((m = pthread_kill(tid, 0)) > 0)
	{
		if(m == ESRCH)
			return 0; 
	}
	return 1; 
}
