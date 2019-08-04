/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/08/04 15:26:33
*	Description:
*
==========================================================================*/

#include <news_reader.h>
sql_conn_pool *sql_pool_create(int min_num, char ip[], int port, char db_name[], char user[], char passwd[])
{
	sql_conn_pool *pSQLPool = NULL;
	if (min_num < 1)
	{
		min_num = 1;
	}

	/* 为连接池划分内存空间 */
	if ((pSQLPool=(sql_conn_pool *)malloc(sizeof(sql_conn_pool))) == NULL)
	{
		return NULL;
	}

	pSQLPool->shutdown    = 0;              //开启连接池
	pSQLPool->pool_number = 0;              //连接数量
	pSQLPool->busy_number = 0;              //正在使用的连接数
	strcpy(pSQLPool->ip, ip);               //数据库IP
	pSQLPool->port = port;                  //数据库Port
	strcpy(pSQLPool->db_name, db_name);     //数据库名字
	strcpy(pSQLPool->user, user);           //使用用户名
	strcpy(pSQLPool->passwd, passwd);       //密码

	/* 创建连接 */
	min_num = min_num > SQL_POOL_NODE_NUM ? SQL_POOL_NODE_NUM : min_num;

	for (int index = 0; index < min_num; index++)
	{
		//创建失败,自定义函数,创建节点函数;
		if (0 != create_db_connect(pSQLPool, &pSQLPool->sql_pool[index]))
		{
			//销毁连接池,自定义函数,销毁连接池;
			sql_pool_destroy(pSQLPool);
			return NULL;
		}
		//创建成功
		pSQLPool->sql_pool[index].index = index;
		pSQLPool->pool_number++;
	}
	return pSQLPool;
}


int create_db_connect(sql_conn_pool *pSQLPool, sql_node_t *pSQLNode)
{
	int opt = 1; //超时时间
	int res = 0; //0正常 -1初始化失败 1 连接失败

	do{
		if(pSQLPool->shutdown == 1)  
			return -1;
		pthread_mutex_init(&pSQLNode->lock, NULL);

		/* 初始化mysql对象 */
		if (NULL == mysql_init(&pSQLNode->fd))
		{
			res = -1;
			break;
		}

		//连接成功
		if(!(pSQLNode->pSQLHandler = mysql_real_connect(
													&pSQLNode->fd, 
													pSQLPool->ip, 
													pSQLPool->user, 
													pSQLPool->passwd, 
													pSQLPool->db_name, 
													pSQLPool->port, 
													NULL, 
													0
												  )))

		{
			pSQLNode->sql_state = DB_DISCONN;
			res = 1;
			break;
		}

		//使用状态与连接状态
		pSQLNode->used = 0;
		pSQLNode->sql_state = DB_CONN;
		//设置自动连接开启
		mysql_options(&pSQLNode->fd, MYSQL_OPT_RECONNECT, &opt);
		opt = 3;
		//设置连接超时时间为3s，3s未连接成功则超时
		mysql_options(&pSQLNode->fd, MYSQL_OPT_CONNECT_TIMEOUT, &opt);
		res = 0;

	}while(0);

	return res;
}

void sql_pool_destroy(sql_conn_pool *pSQLPool)
{
	pSQLPool->shutdown = 1; //关闭连接池
	for (int index=0; index < pSQLPool->pool_number; index++)
	{
		if (NULL != pSQLPool->sql_pool[index].pSQLHandler)
		{
			mysql_close(pSQLPool->sql_pool[index].pSQLHandler);
			pSQLPool->sql_pool[index].pSQLHandler = NULL;
		}
		pSQLPool->sql_pool[index].sql_state = DB_DISCONN; 
		pSQLPool->pool_number--;
	}
}

/*取出一个未使用的连接*/
sql_node_t *get_db_connect(sql_conn_pool *pSQLPool)
{
	//获取一个未使用的连接，用随机值访问index，保证每次访问每个节点的概率基本相同
	int start_index = 0, index = 0, i;
	int ping_res;     

	if (pSQLPool->shutdown == 1)
		return NULL;

	srand((int)time(0)); //根据当前时间生成随机数
	start_index = rand() % pSQLPool->pool_number; //访问的开始地址

	for (i=0; i < pSQLPool->pool_number; i++)
	{
		index = (start_index + i) % pSQLPool->pool_number;

		if (!pthread_mutex_trylock(&pSQLPool->sql_pool[index].lock))
		{
			if (DB_DISCONN == pSQLPool->sql_pool[index].sql_state)
			{
				//重新连接
				if (0 != create_db_connect(pSQLPool, &(pSQLPool->sql_pool[index])))
				{
					//重新连接失败
					release_sql_node(pSQLPool, &(pSQLPool->sql_pool[index]));
					continue;
				}
			}
			//检查服务器是否关闭了连接
			ping_res = mysql_ping(pSQLPool->sql_pool[index].pSQLHandler);
			if (0 != ping_res)
			{
				pSQLPool->sql_pool[index].sql_state = DB_DISCONN;
				release_sql_node(pSQLPool, &(pSQLPool->sql_pool[index]));        //释放连接
			}else{
				pSQLPool->sql_pool[index].used = 1;
				pSQLPool->busy_number++;              //被获取的数量增1
				break ;                         //只需要一个节点
			}
		}
	}

	if (i == pSQLPool->pool_number)
		return NULL;
	else
		return &(pSQLPool->sql_pool[index]);

}

/*归回连接*/
void release_sql_node(sql_conn_pool *pSQLPool, sql_node_t *pSQLNode)
{

	pSQLNode->used = 0;
	pSQLPool->busy_number--;
	pthread_mutex_unlock(&pSQLNode->lock);
}

/*增加或删除连接*/
sql_conn_pool *sql_change_node(sql_conn_pool *pSQLPool, sqlNodeVary mode, int nVaryNum)
{
	int index;        
	int nEndIndex;

	if (mode == INCREASE)
	{
		nEndIndex = pSQLPool->pool_number + nVaryNum;
		/*创建连接*/
		for (index=pSQLPool->pool_number; index < nEndIndex; index++)
		{
			//创建失败
			if (0 != create_db_connect(pSQLPool, &pSQLPool->sql_pool[index]))
			{
				//销毁连接池
				sql_pool_destroy(pSQLPool);
				return NULL;
			}

			//创建成功
			pSQLPool->sql_pool[index].index = index;                                                         
			pSQLPool->pool_number++;
		}
	}else if (mode == DECREASE){	//减少连接
		nEndIndex = pSQLPool->pool_number - nVaryNum -1;
		for (index=pSQLPool->pool_number - 1; index > nEndIndex && index >= 0; index--)
		{       
			if (NULL != pSQLPool->sql_pool[index].pSQLHandler)
			{
				mysql_close(pSQLPool->sql_pool[index].pSQLHandler);
				pSQLPool->sql_pool[index].pSQLHandler = NULL;
			}
			pSQLPool->sql_pool[index].sql_state = DB_DISCONN; 
			pSQLPool->pool_number--;
		}
	}
	return pSQLPool;
}

/*

int main()  
{  
	sql_conn_pool *pSQLPool = sql_pool_create(10, "localhost", 3306, "test", "root", "");  
	sql_node_t *node1  = get_db_connect(pSQLPool);  
	sql_node_t *node2 = get_db_connect(pSQLPool);

	if (NULL == pSQLNode)  
	{  
		printf("get sql pool node error.\n");  
		return -1;  
	} 
	printf("--%d-- \n", node->index);
	printf("busy--%d--\n", node1->busy_number);

	if (mysql_query(&(node->fd), "insert user_info values('wanglongfei023','123','234')"))
	{                                                    
		printf("query error.\n");                                     
		return -1;                                      
	}
	else  
		printf("succeed!\n");  
	release_sql_node(pSQLPool, node);
	release_sql_node(pSQLPool, node2);

	sql_change_node(pSQLPool, DECREASE, 2);//减少
	sql_change_node(pSQLPool, INCREASE, 2);//增加
	sql_pool_destroy(pSQLPool);

	return 0;  
} 
*/
