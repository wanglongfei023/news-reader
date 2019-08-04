/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/05/05 08:03:20
*	Description: 
*
==========================================================================*/

#include<news_reader.h>

void handle_pipe(int sig)
{
	pthread_mutex_lock(&pGlobalPool->lock); 
	--pGlobalPool->thread_alive; 
	--pGlobalPool->thread_busy; 
	pthread_mutex_unlock(&pGlobalPool->lock); 
	LOG("debug: pthread tid:0x%x  exit...\n", (unsigned int)pthread_self()); 
	pthread_exit(0); 	
}

pool_t* thread_pool_create(int max, int min, int que_max)
{

	struct sigaction sa; 
	sa.sa_handler = handle_pipe; 
	sigemptyset(&sa.sa_mask); 
	sa.sa_flags = 0; 
	sigaction(SIGPIPE, &sa, NULL); 

	int i; 
	pool_t* pPool; 
	if((pPool = (pool_t*)malloc(sizeof(pool_t))) == NULL)
	{
		LOG("fatal: fail to create thread pool."); 
		exit(-1);
	}

	pPool->thread_max = max; 
	pPool->thread_min = min; 
	pPool->thread_alive = 0; 
	pPool->thread_busy = 0; 
	pPool->queue_max = que_max; 
	pPool->queue_cur = 0; 
	pPool->shut_down = 1; 
	pPool->queue_front = 0; 
	pPool->queue_rear = 0; 
	pPool->killno = 0; 
	pPool->tids = (pthread_t*)malloc(sizeof(pthread_t)*max); 
	pPool->queue_task = (task_t*)malloc(sizeof(task_t)*que_max); 
	

	if(pPool->tids == NULL || pPool->queue_task == NULL)
	{
		LOG("fatal: fail to create thread pool."); 
		exit(-1);
	}

	for(i = 0; i < max; i++)
	{
		pPool->tids[i] = 0; 
	}
	

	if(pthread_mutex_init(&pPool->lock, NULL) != 0)
	{
		LOG("fatal: fail to create thread pool."); 
		exit(-1);
	}
	
	if(pthread_cond_init(&pPool->full, NULL) != 0)
	{
		LOG("fatal: fail to create thread pool."); 
		exit(-1);
	}

	if(pthread_cond_init(&pPool->empty, NULL) != 0)
	{
		LOG("fatal: fail to create thread pool."); 
		exit(-1);
	}

	if(pthread_attr_init(&pPool->arr) != 0)
	{
		LOG("fatal: fail to create thread pool."); 
		exit(-1);
	}

	if(pthread_attr_setdetachstate(&pPool->arr, PTHREAD_CREATE_DETACHED) != 0)
	{
		LOG("fatal: fail to create thread pool."); 
		exit(-1);
	}


	for(i = 0; i < min; i++)
	{
		pthread_create(&pPool->tids[i], &pPool->arr, thread_worker_func, (void*)pPool); 
		pPool->thread_alive++; 
	}


	pthread_create(&pPool->managertid, &pPool->arr, thread_manager_func, (void*)pPool); 

	return pPool; 

}



void* thread_task_add(pool_t* pPool, void* (*task)(void* arg), void* arg)
{
	
	pthread_mutex_lock(&pPool->lock); 
	while(pPool->queue_cur == pPool->queue_max && pPool->shut_down == 1)
	{
		pthread_cond_wait(&pPool->full, &pPool->lock); 
	}

	if(pPool->shut_down != 1)
	{
		pthread_mutex_unlock(&pPool->lock); 
		pthread_exit(NULL); 
	}

	pPool->queue_task[pPool->queue_front].job = task; 
	pPool->queue_task[pPool->queue_front].arg = (task_t*)arg; 

	pPool->queue_front = (pPool->queue_front + 1)%pPool->queue_max; 
	++pPool->queue_cur; 
	pthread_mutex_unlock(&pPool->lock); 
	LOG("debug: add a task success.");
	pthread_cond_signal(&pPool->empty); 

	return 0; 

}


void* thread_worker_func(void* arg)
{
	pool_t* pool = (pool_t*)arg; 
	task_t task; 
	
	while(pool->shut_down)
	{
		pthread_mutex_lock(&pool->lock); 
		while(pool->queue_cur == 0)
		{
			pthread_cond_wait(&pool->empty, &pool->lock); 
		}

		if(pool->shut_down != 1 || pool->killno > 0)
		{
			--pool->killno; 
			--pool->thread_alive; 
			pthread_mutex_unlock(&pool->lock); 
			pthread_exit(NULL); 
		}

		task.job = pool->queue_task[pool->queue_rear].job; 
		task.arg = pool->queue_task[pool->queue_rear].arg; 
		++pool->thread_busy; 
		--pool->queue_cur; 
		pool->queue_rear = (pool->queue_rear+1)%pool->queue_max; 
		pthread_mutex_unlock(&pool->lock); 
		pthread_cond_signal(&pool->full); 

		(*task.job)(task.arg); 

		pthread_mutex_lock(&pool->lock); 
		--pool->thread_busy; 
		pthread_mutex_unlock(&pool->lock); 

	}
	pthread_exit(NULL); 
}


int thread_pool_destory(pool_t* pool)
{
	int i = 0; 
	pool->shut_down = 0; 
	while(pool->tids[i] != 0)
	{
		pthread_join(pool->tids[i], NULL); 
		i++; 
	}
	pthread_join(pool->managertid, NULL); 
	return 1; 
}
