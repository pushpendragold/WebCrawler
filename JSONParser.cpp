#include "JSONParser.h"
#include <string>
#include <vector>

/* Constructor */
JSONParser::JSONParser(string _path) {
	this->path = _path;
	is_valid = false;
}

/* Check if JSON is valid before performing anything*/
bool JSONParser::isValid() {
	return is_valid;
}

