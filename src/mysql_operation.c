/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/07/26 16:24:57
*	Description:
*
==========================================================================*/

#include <news_reader.h>

/*
MYSQL* connect_database(const char* pHost, const char* pUser, const char* pPasswd, const char* pDB)
{
	MYSQL* pMysqlHander;
	pMysqlHander = mysql_init(NULL);
	if(!mysql_real_connect(pMysqlHander, pHost, pUser, pPasswd, pDB, 0, NULL, 0))
	{
		LOG("warning: fail to connect the database. \
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
*/

int check_user_info(MYSQL* pSQLHandler, const char* pID, const char* pPasswd)
{
	MYSQL_RES* pRes;
	MYSQL_ROW row;
	char szSQL[SQL_LEN] = {0};
	sprintf(szSQL, "select c_passwd from %s where c_mail='%s'", _check_user_table, pID);

	if(mysql_query(pSQLHandler, szSQL))
	{
		LOG("warning: check user info error with wrong sql `%s`\n", szSQL);
		return _sql_exec_error;
	}

	pRes = mysql_use_result(pSQLHandler);
	row = mysql_fetch_row(pRes);
	
	if(row == NULL)
	{
		LOG("notice: user's mail is not exist.[id: %s]", pID);
		return _user_not_exist;
	}

	if(strcmp(row[0], pPasswd) != 0)
	{
		LOG(
			"notice: user password is wrong.[id: %s] [wrong passwd: %s][right passed:%s]\n", 
			pID, 
			pPasswd,
			row[0]
			);
		return _passwd_wrong;
	}

	LOG("notice: user log in success.[id: %s] [password: %s]", pID, pPasswd);	
	return _passwd_right;

	mysql_free_result(pRes);
	
}

int insert_user(MYSQL* pSQLHandler, const char* pID, const char* pName, const char* pPasswd)
{
	char szSQL[SQL_LEN] = {0};

	sprintf(
			szSQL, 
			"insert ignore into %s values('%s','%s','%s', 0)", 
			_check_user_table, 
			pID, 
			pName, 
			pPasswd
		   );

	if(mysql_query(pSQLHandler, szSQL))
	{
		LOG(
			"warning: fail to insert a user. err_msg: user id:%s user name:%s password:%s",
			pID,
			pName,
			pPasswd
		   );
		return _sql_exec_error;
	}

	sprintf(
			szSQL, 
			"insert ignore into %s(c_mail, u_name) values('%s','%s')", 
			_user_info_table, 
			pID, 
			pName 
		   );

	if(mysql_query(pSQLHandler, szSQL))
	{
		LOG(
			"warning: fail to insert a user. err_msg: user id:%s user name:%s",
			pID,
			pName
		   );
		return _insert_error;
	}

	bzero(szSQL, sizeof(szSQL));


	LOG("%s", "insert success.");
	return _insert_success;
}

int get_user_info(MYSQL* pSQLHandler, const char* pID, user_info* pUserInfo)
{
	MYSQL_RES* pRes;
	MYSQL_ROW row;
	char szSQL[SQL_LEN] = {0};
	sprintf(szSQL, "select * from %s where c_mail='%s'", _user_info_table, pID);

	if(mysql_query(pSQLHandler, szSQL))
	{
		LOG("warning: select error with wrong sql `%s`\n", szSQL);
		return _sql_exec_error;
	}

	pRes = mysql_use_result(pSQLHandler);
	row = mysql_fetch_row(pRes);
	
	if(row == NULL)
	{
		return _user_not_exist;
	}

	if(row[1] != NULL)
		strcpy(pUserInfo->szMail, row[1]);

	if(row[2] != NULL)
		strcpy(pUserInfo->szName, row[2]);

	pUserInfo->nSex = row[3] == NULL ? -1 : atoi(row[3]);

	pUserInfo->nAge = row[4] == NULL ? -1 : atoi(row[4]);

	if(row[5] != NULL)
		strcpy(pUserInfo->szPhoto, row[5]);

	if(row[6] != NULL)
		strcpy(pUserInfo->szArea, row[6]);

	if(row[7] != NULL)
		strcpy(pUserInfo->szTag, row[7]);

	pUserInfo->nMember = atoi(row[8]);

	return _get_info_success;
	mysql_free_result(pRes);

}


int delete_user(MYSQL* pSQLHandler, const char* pID)
{
	char szSQL[SQL_LEN] = {0};
	sprintf(szSQL, "delete from %s where c_mail='%s'", _check_user_table, pID);
	if(mysql_query(pSQLHandler, szSQL))
	{
		LOG("warning: fail to delete a user. err_msg: `sql`:%s\n", szSQL);
		return _sql_exec_error;
	}
	return TRUE;
}

/*
int main(int argc,char** argv)
{
	MYSQL* pSQLHandler = connect_database("localhost", "root", "", "news_reader");
	insert_user(pSQLHandler, "395592722@qq.co", "wanglongfei", "123456");
	check_user_info(pSQLHandler, "395592722@qq.co", "123456");
	//delete_user(pSQLHandler, "395592722@qq.co");
	return 0;
}
*/
