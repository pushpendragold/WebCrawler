#ifndef _H_WEB_SPIDER_H
#define _H_WEB_SPIDER_H

#include <string>
#include <map>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>

using namespace std;

#include "WebSpider-conf.h"
#include "DBConnection.h"
#include "MySQL.h"
#include "LogTrace.h"

#define TEST_SUIT 1

enum DB_TYPE {
	MYSQL_DB, POSTGRE
};

class WebSpider {

private:

	/* init Base URL */
	string baseURL;

	/* Holds current URL in processing */
	string currentURL;

	/* Depth */
	int depth;

	/* URL limit */
	int limit;

	/* Processed URLs */
	int processedURLs;

	/* Run on single page */
	bool runOnSinglePage;

	/* Show status */
	bool showStatus;

	/* Drop table if already exits */
	bool forceCreateTable;

	/* Log & Trace files */
	string visitedURLlogFile;

	/* Base domain name */
	string baseWebsiteURL;

	/* Event Handlers */
	map<string, p_handler> actionHandlers;

	/* Processed & Queued URLs */
	map<string, int> listProcessedURLs;

	/* Queue of URLs processed */
	queue<string> Q;

	/* Read data for tags */
	vector<HtmlDataTag> saveHTMLRule;

	/* Replace HTML tags with user defined string */
	vector<ReplaceHTMLTag> replaceHTMLStrList;

	/* Save images for img tag contains */
	vector<IMGDownloadTag> saveImgRule;
	string saveImagePath;
	int imgCount;

	/* HTML tag based rules */
	vector<CommonHTMLDataTag> saveMultiHTMLRules;

	/* Ignore base urls pattern */
	vector<string> ignoreURLPatterns;

	/* Traces */
	TRACE trc;

	/* Delay between each URL Call in seconds */
	int delaySeconds;

	/* DB */
	DBConnection * db;

public:

	/* Construction */
	WebSpider(string _URL, int _depth, int DB_TYPE);
	WebSpider();

	/* Destructor */
	~WebSpider();

	/* init */
	void Init();

	/* Time */
	inline void setDelayBtwEachCall(int _delay) {
		delaySeconds = _delay;
	}
	inline int getDelayBtwEachCall() {
		return delaySeconds;
	}

	/* Update Base URL*/
	inline void UpdateBaseURL(string _nURL) {
		baseURL = _nURL;
	}

	/* Update Depth */
	inline void UpdateDepth(int _nDepth) {
		depth = _nDepth;
	}

	/* Get current number of URLs Processed */
	inline int GetProcessedURLsNum() {
		return processedURLs;
	}

	/* Update visited URL Log file */
	inline void UpdateVisitedURLfileName(string _nName) {
		visitedURLlogFile = _nName;
	}

	/* Enable/Disable Traces */
	inline void EnableTrace(bool _enable, int _traceLevel) {
		trc.EnableTrace(_enable, _traceLevel);
		if (db)
			db->EnableTrace(_enable, _traceLevel);
		else
			cout << "db not created " << endl;
	}

	/* Set Table Name */
	inline void SetTableName(string _table) {
		db->SetTableName(_table);
	}

	inline void ConnectToServer() {
		db->ConnectToServer();
	}

	void Init_DBOptions(string host, string user, string pass, string database,
			int port) {
		if (db)
			db->Init_Connection(host, user, pass, database, port);
	}

	string SetEscapeSequense(string input);

	/* Show Status */
	inline void SetShowStatus(bool _enable = true) {
		showStatus = _enable;
	}

	/* Run */
	bool Run();

	/* Run for single page - To check if all conditions executed properly */
	bool RunSinglePage(string _url);

	/* To add tags whose content need to be retreived */
	bool AddHTMLDataRule(string _ruleName, string _fullHTMLTag, bool mustHave =
			true, int size = 1024);

	/* To add HTML within HTML rules, this rule can have multiple HTML rules for single HTML */
	bool AddSubHTMLDataRule(HtmlDataTag _BaseHTMLTag,
			vector<SubHTMLDataTag> &subHTML);

	/* To Save image for given tag in img HTML tag */
	bool AddSaveIMGRule(string _ruleName, string _findInImg);

	/* Add URL to visited list */
	bool AddURLToVisitedList(string url);

	/* Add URL to ignore pattern URLs list */
	bool AddIgnorePatternURLs(string url);

	/* Replace HTML tags with some user defined string */
	void ReplaceHTMLTagWith(string _htmlTag, string _replacewith,
			string _forHTMLRule);

	string RunReplaceHTMLRules_SingleRule(int i, string &data);

	/* Do Exit Tasks */
	void DoExitTask();

	/* update save image path */
	inline void SetImageDownloadLocation(string _path) {
		saveImagePath = _path;
	}

	/* Set limit on number of URLs */
	inline void SetLimit(int _limit) {
		limit = _limit;
	}

	/* data received from curl */
	static string curlData;

private:

	/* ************************
	 *  HTML processing
	 * ************************/

	/* Get Web Page */
	bool GetWebPage(string _url);

	/* Download Image */
	bool DownloadImage(string _imgPath, string _saveToPath,
			string _saveFileName);
	string GetImagePathForRule(IMGDownloadTag &rule, string &data);
	bool IsImageTag(int pos, string &data, string &completeTag);
	string GetImageSrc(string &completeTag);
	string GetRandomImageName(string &imgPath);

	/* Run Replace HTML rules */
	string RunReplaceHTMLRules(string &_data, string &_htmlRuleName);

	/* Get OutBound Links */
	void GetOutBoundLinks(vector<string> &_outBound, string &_data);
	string GetURLFromAHref(size_t pos, string &data);

	/* Validate URL */
	bool CheckAndValidateURL(string &url);

	/* Check if html code at position is not commented out */
	bool CheckIfCodeNotCommentedOut(int pos, string &data);

	/* Remove HTML tags */
	string StripHTML(string &data, bool removeOtherChar = false);
	bool CheckOtherChar(char t);

	/* Read data between html tag */
	bool GetHTMLDataForTag(string &fullTag, string &tag, const string &data,
			vector<string> &ans);
	string GetHTMLDataForTagPos(string &tag, const string &data, size_t pos);
	string GetClosingTag(string &tag);
	string GetBaseTag(string &tag);
	string GetCompleteTagAtPosition(const string &data, int pos);

	/* ************************
	 *  DUMP / Restore
	 * ************************/

	/* Dump Queue to a file */
	void DumpQueue();

	/* Dump processed URL to a file */
	void DumpStatus();

public:
	/* Load Queue */
	void LoadQueue(string file);

	/* Load Processed URL from file */
	void LoadStatus(string file);
private:
	/* ************************
	 *  Log visited URLs list
	 * ************************/
	ofstream logfile;
	void OpenLogFile() {
		logfile.open(visitedURLlogFile.c_str(),
				std::fstream::in | std::fstream::out | std::fstream::app);
	}
	void LogURL(string _url) {
		OpenLogFile();
		logfile << _url << endl;
		CloseLogFile();
	}
	void CloseLogFile() {
		logfile.close();
	}

	/* ******************************
	 *      Utility functions
	 * ******************************/

	/* Get file name from URL */
	string GetFileNameFromURL(string _path);

	/* ***********************************
	 *
	 *  Test Suit for utility functions
	 *         & Flow test
	 *
	 * ***********************************/
#ifdef TEST_SUIT

public:
	inline string Test_StripHTML(string &data) {
		return StripHTML(data);
	}
	inline string Test_GetFileNameFromURL(string &data) {
		return GetFileNameFromURL(data);
	}
	inline bool Test_DownloadImage(string &_imgPath, string &_saveToPath,
			string &_saveFileName) {
		return DownloadImage(_imgPath, _saveToPath, _saveFileName);
	}
	inline bool Test_ConnectToServer() {
		return db->IsConnected();
	}
#endif

public:
	/*******************
	 *   TODO - Start
	 *******************/

	/* Easy Configuration */
	bool Easy_Configuration(int _opt, int val);
	bool Easy_Configuration(int _opt, string val);

	/* Register Event Handler */
	void RegisterEventHandler(char * _eventName, int _eventType,
			p_handler _function);

	/* DeRegister Event Handler */
	void DeRegisterEventHandler(char * _eventName, int _eventType);

	/* Register Signal Handler */
	void RegisterSignalHandler();
	/*******************
	 *   TODO - END
	 *******************/

	bool isForceCreateTable() const {
		return forceCreateTable;
	}

	void setForceCreateTable(bool forceCreateTable) {
		this->forceCreateTable = forceCreateTable;
	}

	const string& getBaseWebsiteUrl() const {
		return baseWebsiteURL;
	}

	void setBaseWebsiteUrl(const string& baseWebsiteUrl) {
		baseWebsiteURL = baseWebsiteUrl;
	}

	bool getRunOnSinglePage() const {
		return runOnSinglePage;
	}

	void setRunOnSinglePage(bool runOnSinglePage) {
		this->runOnSinglePage = runOnSinglePage;
	}
};

/* CURL data read function */
size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up);
/* CURL data write function */
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);

#endif
