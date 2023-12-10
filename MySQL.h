#ifndef _H_MY_SQL_H_
#define _H_MY_SQL_H_

#include <mysql/mysql.h> 

#include "DBConnection.h"
#include "MySQL.h"
#include "WebSpider-conf.h"
#include "LogTrace.h"

class MySqlConnection: public DBConnection {
public:

	MySqlConnection();
	virtual ~MySqlConnection();

	bool ConnectToServer();
	bool CreateTable(vector<HtmlDataTag> &saveHTMLRule,
			vector<IMGDownloadTag> &saveImgRule,
			vector<CommonHTMLDataTag> &saveMultiHTMLRules);
	bool DropTable(string table);
	bool ExecuteQuery(string query);
	string GetInsertStatement(vector<HtmlDataTag> &saveHTMLRule,
			vector<IMGDownloadTag> &saveImgRule,
			vector<CommonHTMLDataTag> &saveMultiHTMLRules);
	void CloseConnection();
	void Init_Connection(string host, string user, string pass,
			string db, int port);
	int NumberOfRecords(string tableName);

private:

	MYSQL mysql;
	string mysql_host; /* mysql server   */
	string mysql_username; /* mysql username */
	string mysql_password; /* mysql password */
	string mysql_db; /* mysql db */
	int mysql_port; /* mysql port     */

};

#endif
