#ifndef _H_LOG_TRACE_H_H
#define _H_LOG_TRACE_H_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

enum TRC {
	INFO = 1, DBG, ERR, ALL
};

/* Trace file name */
#define TRACE_FILE "trace"

class TRACE {
private:
	/* Trace */
	bool enableTrace;
	int traceLevel;
	ofstream tracefile;

public:
	TRACE() {
		enableTrace = false;
		traceLevel = 0;
	}
	~TRACE() {
	}

	inline void EnableTrace(bool _enable, int _traceLevel) {
		enableTrace = _enable;
		traceLevel = _traceLevel;
	}
	inline void OpenTraceFile() {
		tracefile.open(TRACE_FILE,
				std::fstream::in | std::fstream::out | std::fstream::app);
	}
	inline void CloseTraceFile() {
		tracefile.close();
	}
	void LogTrace(string data, int _type);
	string GetType(int _type);
	string GetCurrentTime();

};

#endif
