#ifndef _H_POST_GRE_SQL_H_
#define _H_POST_GRE_SQL_H_

#include <pqxx/pqxx> 

#include "DBConnection.h"
#include "WebSpider-conf.h"
#include "LogTrace.h"

using namespace pqxx;

class PGSQLConnection: public DBConnection {
public:

	PGSQLConnection();
	virtual ~PGSQLConnection();

	bool iSsingleQuotesQuery() {
		return true;
	}

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

	connection *conn;

	string pg_host; /* server   */
	string pg_username; /* username */
	string pg_password; /* password */
	string pg_db; /* db */
	int pg_port; /* port */

};

#endif
