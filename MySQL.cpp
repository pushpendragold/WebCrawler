#include <cstdio>
#include <iostream>
#include <mysql/mysql.h> 
#include <vector>
#include <string>
#include <cstdlib>

#include "MySQL.h"
#include "LogTrace.h"
#include "WebSpider-conf.h"

using namespace std;

MySqlConnection::MySqlConnection() {
	mysql_port = 0;
}
MySqlConnection::~MySqlConnection() {

}
/***************************************************************
 *  MySqlConnection::CreateTable
 *  To Create Table - based on rules sets before starting setup
 *  
 *  Parameters :
 *          string query (in)
 *  Return :
 *          bool status (out)
 * 
 ***************************************************************/
bool MySqlConnection::CreateTable(vector<HtmlDataTag> &saveHTMLRule,
		vector<IMGDownloadTag> &saveImgRule,
		vector<CommonHTMLDataTag> &saveMultiHTMLRules) {
	if (!isConnected)
		return false;
	string Query("CREATE TABLE " + tableName + " ( URL VARCHAR(1024) ");

	if (saveHTMLRule.size()) {
		for (int i = 0; i < saveHTMLRule.size(); ++i) {
			char input[100];
			sprintf(input, "%d", saveHTMLRule[i].FieldSize);
			string Field(input);

			Query += ", " + saveHTMLRule[i].ruleName + " VARCHAR(" + Field
					+ ")";
		}
	}

	if (saveMultiHTMLRules.size()) {
		for (int j = 0; j < saveMultiHTMLRules.size(); ++j) {
			for (int i = 0; i < saveMultiHTMLRules[j].subRules.size(); ++i) {
				char input[100];
				sprintf(input, "%d", saveMultiHTMLRules[j].baseTag.FieldSize);
				string Field(input);

				Query += ", " + saveMultiHTMLRules[j].subRules[i].ruleName
						+ " VARCHAR(" + Field + ")";
			} // Sub rule in each multi rule.
		} // Multi Rule
	}

	if (saveImgRule.size()) {
		for (int i = 0; i < (int) saveImgRule.size(); ++i)
			Query += ", " + saveImgRule[i].ruleName + " VARCHAR(1024)";
	}
	Query += ")";

	ostringstream trace;
	trace << "MySqlConnection::CreateTable Query Generated : " << Query << endl;
	trc.LogTrace(trace.str(), INFO);

	return ExecuteQuery(Query);
}
/***************************************************************
 *  MySqlConnection::DropTable
 *  To Create Table - drop table
 *  
 *  Parameters :
 *          string _table (in)
 *  Return :
 *          bool status (out)
 * 
 ***************************************************************/
bool MySqlConnection::DropTable(string _table) {
	if (!isConnected)
		return false;

	if (!_table.size()) {
		ostringstream trace;
		trace << "MySqlConnection::DropTable table name not specified :"
				<< endl;
		trc.LogTrace(trace.str(), ERR);
		return false;
	}

	string Query("DROP TABLE " + _table);
	return ExecuteQuery(Query);
}
/***************************************************************
 *  MySqlConnection::ExecuteQuery
 *  To Execute Query
 *  
 *  Parameters :
 *          string query (in)
 *  Return :
 *          bool status (out)
 * 
 ***************************************************************/
bool MySqlConnection::ExecuteQuery(string query) {
	if (!isConnected)
		return false;
	return mysql_query(&mysql, query.c_str()) == 0 ? true : false;
}
/***************************************************************
 *  MySqlConnection::ConnectToMysqlServer
 *  To Connect to MySQL host
 *  
 *  Parameters :
 *          Nothing
 *  Return :
 *          bool status (out)
 * 
 ***************************************************************/
bool MySqlConnection::ConnectToServer() {
	/* mysql init */
	mysql_init(&mysql);

	/* Connect to mysql */
	if (!mysql_real_connect(&mysql, mysql_host.c_str(), mysql_username.c_str(),
			mysql_password.c_str(), mysql_db.c_str(), mysql_port,
			NULL, 0)) {
		ostringstream trace;
		trace
				<< "MySqlConnection::ConnectToMysqlServer Failed to connect to DB because of Error : "
				<< mysql_error(&mysql) << endl;
		trc.LogTrace(trace.str(), ERR);
		isConnected = false;
		return false;
	}
	ostringstream trace;
	trace << "MySqlConnection::ConnectToMysqlServer Mysql Server is connected."
			<< endl;
	trc.LogTrace(trace.str(), INFO);
	isConnected = true;
	/* Connected */
	return true;
}

/***************************************************************
 *  MySqlConnection::GetInsertStatement
 *  Get Instert Query template till
 *  INSERT INTO table (field1,...) VALUES (
 *  
 *  Parameters :
 *          Nothing
 *  Return :
 *          string template
 * 
 ***************************************************************/
string MySqlConnection::GetInsertStatement(vector<HtmlDataTag> &saveHTMLRule,
		vector<IMGDownloadTag> &saveImgRule,
		vector<CommonHTMLDataTag> &saveMultiHTMLRules) {
	string ans("INSERT INTO ");
	ans += tableName;
	ans += "( URL ";

	if (saveHTMLRule.size()) {
		for (int i = 0; i < saveHTMLRule.size(); ++i)
			ans += ", " + saveHTMLRule[i].ruleName;
	}

	if(saveMultiHTMLRules.size()) {
		for(int i = 0; i < saveMultiHTMLRules.size(); ++i) {
			for(int j = 0; j < saveMultiHTMLRules[i].subRules.size(); ++j) {
				ans += ", " + saveMultiHTMLRules[i].subRules[j].ruleName;
			}
		}
	}

	if (saveImgRule.size()) {
		for (int i = 0; i < saveImgRule.size(); ++i)
			ans += ", " + saveImgRule[i].ruleName;
	}

	ans += " ) VALUES ( ";

	ostringstream trace;
	trace << "MySqlConnection::GetInsertStatement [" << ans << "]" << endl;
	trc.LogTrace(trace.str(), INFO);

	return ans;
}
void MySqlConnection::CloseConnection() {
	if (IsConnected())
		mysql_close(&mysql);
}
void MySqlConnection::Init_Connection(string host, string user, string pass,
		string db, int port) {
	mysql_host = host;
	mysql_username = user;
	mysql_password = pass;
	mysql_db = db;
	mysql_port = port;
}

/***************************************************************
 *  MySqlConnection::NumberOfRecords
 *  Get number of records in the table specified.
 *   
 *  Parameters :
 *          string tableName(in)
 *  Return :
 *          int numberOfRecords (out)
 *  
 *  TO-TEST ????
 * 
 ***************************************************************/
int MySqlConnection::NumberOfRecords(string tableName) {
	MYSQL_ROW row;
	MYSQL_RES *result;
	unsigned long *lengths;

	string Query("SELECT COUNT(*) FROM " + tableName);
	int res = 0;

	if (mysql_query(&mysql, Query.c_str())) {
		ostringstream trace;
		trace << "MySqlConnection::NumberOfRecords Table [" << tableName
				<< "] not found " << endl;
		trc.LogTrace(trace.str(), INFO);

		return -1;
	} else {
		result = mysql_store_result(&mysql);
		row = mysql_fetch_row(result);
		lengths = mysql_fetch_lengths(result);

		char nRows[256];
		sprintf(nRows, "[%.*s]", (int) lengths[0], row[0]);
		res = atoi(nRows);
	}
	return res;
}
