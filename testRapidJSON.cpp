// rapidjson/example/simpledom/simpledom.cpp`
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"
#include <fstream>

#include <iostream>
using namespace rapidjson;
using namespace std;

int main() {
	ifstream ifs("input.json");
	IStreamWrapper isw(ifs);

	Document d;
    d.ParseStream(isw);

    Value &s = d["baseURL"];
    cout << s.GetString() << endl;

    return 0;
}
