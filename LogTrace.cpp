#include <cstdio>
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>

#include "LogTrace.h"

void TRACE::LogTrace(string data, int _type) {
	if (enableTrace && (_type == traceLevel || traceLevel == ALL)) {
		OpenTraceFile();
		tracefile << GetCurrentTime() << " " << GetType(_type) << data;
		CloseTraceFile();
	}
}
string TRACE::GetType(int _type) {
	switch (_type) {
	case INFO:
		if (traceLevel == INFO || traceLevel == ALL) {
			string res("INF: ");
			return res;
		}
		break;
	case DBG:
		if (traceLevel == DBG || traceLevel == ALL) {
			string res("DBG: ");
			return res;
		}
		break;
	case ERR:
		if (traceLevel == ERR || traceLevel == ALL) {
			string res("ERR: ");
			return res;
		}
		break;
	}
}
string TRACE::GetCurrentTime() {
	// current date/time based on current system
	time_t now = time(0);

	// convert now to string form
	string ret(ctime(&now));
	string ans = ret.substr(0, ret.size() - 1); // removing last new line char

	//string ret(dt);
	//free(dt);

	return ans;
}

