#ifndef _H_DB_CONNECTION_H_
#define _H_DB_CONNECTION_H_

#include <iostream>
#include <string>
#include <cstdio>

#include "LogTrace.h"
#include "WebSpider-conf.h"

using namespace std;

class DBConnection {
protected:
	string tableName;
	TRACE trc;
	bool isConnected;

public:

	virtual ~DBConnection() {
	}
	;

	virtual bool ConnectToServer() = 0;
	virtual bool CreateTable(vector<HtmlDataTag> &saveHTMLRule,
			vector<IMGDownloadTag> &saveImgRule,
			vector<CommonHTMLDataTag> &saveMultiHTMLRules) = 0;
	virtual bool DropTable(string table) = 0;
	virtual bool ExecuteQuery(string query) = 0;
	virtual string GetInsertStatement(vector<HtmlDataTag> &saveHTMLRule,
			vector<IMGDownloadTag> &saveImgRule,
			vector<CommonHTMLDataTag> &saveMultiHTMLRules) = 0;
	virtual void CloseConnection() = 0;
	virtual void Init_Connection(string host, string user, string pass,
			string db, int port) = 0;
	virtual int NumberOfRecords(string tableName) = 0;

	// Returns if database query requires single quotes default is double quotes
	virtual bool iSsingleQuotesQuery() {
		return false;
	}
	virtual bool IsConnected() {
		return isConnected;
	}
	virtual void SetTableName(string _tName) {
		tableName = _tName;
	}
	virtual int CheckTableName() {
		return tableName.size();
	}
	virtual void EnableTrace(bool _enable, int _traceLevel) {
		trc.EnableTrace(_enable, _traceLevel);
	}
	virtual string GetTableName() {
		return tableName;
	}
};

#endif
