/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/07/26 16:24:57
*	Description:
*
==========================================================================*/

#include <news_reader.h>

MYSQL* connect_database(const char* pHost, const char* pUser, const char* pPasswd, const char* pDB)
{
	MYSQL* pMysqlHander;
	pMysqlHander = mysql_init(NULL);
	if(!mysql_real_connect(pMysqlHander, pHost, pUser, pPasswd, pDB, 0, NULL, 0))
	{
		printf("warning: fail to connect the database. \
				\nerr_msg: host name:%s user:%s password:%s database:%s",
				pHost,
				pUser,
				pPasswd,
				pDB
			   );
		return NULL;
	}
	return pMysqlHander;
}

int check_user_info(MYSQL* pSQLHandler, const char* pID, const char* pPasswd)
{
	MYSQL_RES* pRes;
	MYSQL_ROW row;
	char szSQL[SQL_LEN] = {0};
	sprintf(szSQL, "select c_passwd from %s where c_mail='%s'", _check_user_table, pID);

	if(mysql_query(pSQLHandler, szSQL))
	{
		printf("warning: select error with wrong sql `%s`", szSQL);
		mysql_close(pSQLHandler);
		return -1;
	}

	pRes = mysql_use_result(pSQLHandler);
	row = mysql_fetch_row(pRes);
	
	if(row == NULL)
	{
		printf("notice: user log in failed and id is not exist.[id: %s]", pID);
		return _user_not_exist;
	}

	if(strcmp(row[0], pPasswd) != 0)
	{
		printf("notice: user log in failed.[id: %s] [password: %s]\n", pID, pPasswd);
		return _passwd_wrong;
	}
	printf("notice: user log in success.[id: %s] [password: %s]", pID, pPasswd);	
	return _passwd_right;

	mysql_free_result(pRes);
	//mysql_close(pSQLHandler);
	
}

int insert_user(MYSQL* pSQLHandler, const char* pID, const char* pName, const char* pPasswd)
{
	char szSQL[SQL_LEN] = {0};
	sprintf(szSQL, "insert ignore into %s values('%s','%s','%s')", _check_user_table, pID, pName, pPasswd);
	if(mysql_query(pSQLHandler, szSQL))
	{
		printf(
				"warning: fail to insert a user. err_msg: user id:%s user name:%s password:%s",
				pID,
				pName,
				pPasswd
			   );
		mysql_close(pSQLHandler);
		return -1;
	}
	printf("%s", "insert success.");
	return 1;
}

int delete_user(MYSQL* pSQLHandler, const char* pID)
{
	char szSQL[SQL_LEN] = {0};
	sprintf(szSQL, "delete from %s where c_mail='%s'", _check_user_table, pID);
	if(mysql_query(pSQLHandler, szSQL))
	{
		printf("warning: fail to delete a user. err_msg: `sql`:%s\n", szSQL);
		mysql_close(pSQLHandler);
		return -1;
	}
	return 1;
}

/*
int main(int argc,char** argv)
{
	MYSQL* pSQLHandler = connect_database("localhost", "root", "", "news_reader");
	//insert_user(pSQLHandler, "395592722@qq.co", "wanglongfei", "123456");
	check_user_info(pSQLHandler, "395592722@qq.co", "123456");
	delete_user(pSQLHandler, "395592722@qq.co");
	return 0;
}

*/
