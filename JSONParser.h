#ifndef _H_JSON_PARSER_H_
#define _H_JSON_PARSER_H_

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"
#include <fstream>
#include "JSONConstants.h"

using namespace std;
using namespace rapidjson;

/*
 * Utility class for parsing JSON file.
 *
 */
class JSONParser {

private:
	Document d;
    string path; /* JSON Path */
    bool is_valid;
public:
    JSONParser(string path);
	bool isValid();
};

#endif
