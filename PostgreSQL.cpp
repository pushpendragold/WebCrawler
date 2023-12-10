#include <cstdio>
#include <iostream>

#include <pqxx/pqxx> 

#include <vector>
#include <string>

#include "PostgreSQL.h"

#include "LogTrace.h"
#include "WebSpider-conf.h"

using namespace std;
using namespace pqxx;

PGSQLConnection::PGSQLConnection() {
	conn = NULL;
	pg_port = 0;
}
PGSQLConnection::~PGSQLConnection() {
	conn->disconnect();
	delete conn;
	conn = NULL;
}
/***************************************************************
 *  PGSQLConnection::CreateTable
 *  To Create Table - based on rules sets before starting setup
 *  
 *  Parameters :
 *          string query (in)
 *  Return :
 *          bool status (out)
 * 
 ***************************************************************/
bool PGSQLConnection::CreateTable(vector<HtmlDataTag> &saveHTMLRule,
		vector<IMGDownloadTag> &saveImgRule,
		vector<CommonHTMLDataTag> &saveMultiHTMLRules) {
	if (!isConnected)
		return false;

	string Query("CREATE TABLE " + tableName + " ( URL VARCHAR(512) ");

	if (saveHTMLRule.size()) {
		for (unsigned int i = 0; i < saveHTMLRule.size(); ++i) {
			char input[100];
			sprintf(input, "%d", saveHTMLRule[i].FieldSize);
			string Field(input);

			Query += ", " + saveHTMLRule[i].ruleName + " VARCHAR(" + Field
					+ ")";
		}
	}

	if (saveMultiHTMLRules.size()) {
		for (unsigned int j = 0; j < saveMultiHTMLRules.size(); ++j) {
			for (unsigned int i = 0; i < saveMultiHTMLRules[j].subRules.size();
					++i) {
				char input[100];
				sprintf(input, "%d", saveMultiHTMLRules[j].baseTag.FieldSize);
				string Field(input);

				Query += ", " + saveMultiHTMLRules[j].subRules[i].ruleName
						+ " VARCHAR(" + Field + ")";
			} // Sub rule in each multi rule.
		} // Multi Rule
	}

	if (saveImgRule.size()) {
		for (unsigned int i = 0; i < saveImgRule.size(); ++i) {
			Query += ", " + saveImgRule[i].ruleName + " VARCHAR(512)";
		}
	}
	Query += ")";

	ostringstream trace;
	trace << "PGSQLConnection::CreateTable Query Generated : " << Query << endl;
	trc.LogTrace(trace.str(), INFO);

	return ExecuteQuery(Query);
}
/***************************************************************
 *  PGSQLConnection::DropTable
 *  To Create Table - drop table
 *  
 *  Parameters :
 *          string _table (in)
 *  Return :
 *          bool status (out)
 * 
 ***************************************************************/
bool PGSQLConnection::DropTable(string _table) {
	if (!isConnected)
		return false;

	if (!_table.size()) {
		ostringstream trace;
		trace << "PGSQLConnection::DropTable table name not specified :"
				<< endl;
		trc.LogTrace(trace.str(), ERR);
		return false;
	}

	string Query("DROP TABLE " + _table);
	return ExecuteQuery(Query);
}
/***************************************************************
 *  PGSQLConnection::ExecuteQuery
 *  To Execute Query
 *  
 *  Parameters :
 *          string query (in)
 *  Return :
 *          bool status (out)
 * 
 ***************************************************************/
bool PGSQLConnection::ExecuteQuery(string query) {
	if (!isConnected)
		return false;

	try {
		/* Create a transactional object. */
		work W(*conn);

		/* Execute SQL query */
		W.exec(query.c_str());
		W.commit();

		return true;
	} catch (const std::exception &e) {
		ostringstream trace;
		trace << "PGSQLConnection::ExecuteQuery Failed :" << e.what() << endl;
		trc.LogTrace(trace.str(), ERR);
		return false;
	}
}
/***************************************************************
 *  PGSQLConnection::ConnectToMysqlServer
 *  To Connect to MySQL host
 *  
 *  Parameters :
 *          Nothing
 *  Return :
 *          bool status (out)
 * 
 ***************************************************************/
bool PGSQLConnection::ConnectToServer() {
	try {
		string ConnectionStr;
		ConnectionStr = "dbname=" + pg_db + " user=" + pg_username
				+ " password=" + pg_password + " hostaddr=" + pg_host + " port="
				+ to_string(pg_port);
		conn = new connection(ConnectionStr);
		if (conn->is_open()) {
			isConnected = true;
			ostringstream trace;
			trace
					<< "PGSQLConnection::ConnectToServer [Connection established]";
			trc.LogTrace(trace.str(), INFO);
			return true;
		} else {
			ostringstream trace;
			trace << "PGSQLConnection::ConnectToServer [Connection Failed]";
			trc.LogTrace(trace.str(), ERR);
			return false;
		}
	} catch (const std::exception &e) {
		cerr << e.what() << std::endl;
		ostringstream trace;
		trace << "PGSQLConnection::ConnectToServer [Connection Failed "
				<< e.what() << "]";
		trc.LogTrace(trace.str(), ERR);
		return false;
	}
}

/***************************************************************
 *  PGSQLConnection::GetInsertStatement
 *  Get Instert Query template till
 *  INSERT INTO table (field1,...) VALUES (
 *  
 *  Parameters :
 *          Nothing
 *  Return :
 *          string template
 * 
 ***************************************************************/
string PGSQLConnection::GetInsertStatement(vector<HtmlDataTag> &saveHTMLRule,
		vector<IMGDownloadTag> &saveImgRule,
		vector<CommonHTMLDataTag> &saveMultiHTMLRules) {
	string ans("INSERT INTO ");
	ans += tableName;
	ans += "( URL ";

	if (saveHTMLRule.size()) {
		for (unsigned int i = 0; i < saveHTMLRule.size(); ++i)
			ans += ", " + saveHTMLRule[i].ruleName;
	}

	if (saveMultiHTMLRules.size()) {
		for (int i = 0; i < saveMultiHTMLRules.size(); ++i) {
			for (int j = 0; j < saveMultiHTMLRules[i].subRules.size(); ++j) {
				ans += ", " + saveMultiHTMLRules[i].subRules[j].ruleName;
			}
		}
	}

	if (saveImgRule.size()) {
		for (unsigned int i = 0; i < saveImgRule.size(); ++i)
			ans += ", " + saveImgRule[i].ruleName;
	}

	ans += " ) VALUES ( ";

	ostringstream trace;
	trace << "PGSQLConnection::GetInsertStatement [" << ans << "]" << endl;
	trc.LogTrace(trace.str(), INFO);

	return ans;
}
void PGSQLConnection::CloseConnection() {
	if (IsConnected() && conn) {
		conn->disconnect();
		conn->disconnect();
		delete conn;
		conn = NULL;
	}

}
void PGSQLConnection::Init_Connection(string host, string user, string pass,
		string db, int port) {
	pg_host = host;
	pg_username = user;
	pg_password = pass;
	pg_db = db;
	pg_port = port;
}
/***************************************************************
 *  PGSQLConnection::NumberOfRecords
 *  Get number of records in the table specified.
 *   
 *  Parameters :
 *          string tableName(in)
 *  Return :
 *          int numberOfRecords (out)
 *  
 ***************************************************************/
int PGSQLConnection::NumberOfRecords(string tableName) {
	string Query("SELECT COUNT(*) FROM " + tableName);
	int res = 0;

	try {
		/* Create a non-transactional object. */
		nontransaction N(*conn);

		/* Execute SQL query */
		result R(N.exec(Query.c_str()));

		/* List down all the records */
		for (result::const_iterator c = R.begin(); c != R.end(); ++c)
			res = c[0].as<int>();
	} catch (const std::exception &e) {
		cerr << e.what() << std::endl;
		ostringstream trace;
		trace << "PGSQLConnection::NumberOfRecords [Connection Failed "
				<< e.what() << "]";
		trc.LogTrace(trace.str(), ERR);
		return -1;
	}

	return res;
}

