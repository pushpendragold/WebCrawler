#include <curl/curl.h> 
#include <sstream>
#include <cassert>
#include <iostream>
#include <cstdio>
#include <vector>
#include <stack>
#include <queue>
#include <fstream>
#include <map>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <ctime>
#include <string.h>
#include <mysql/mysql.h> 

#include "WebSpider.h"
#include "MySQL.h"
#include "PostgreSQL.h"

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__)
#include <windows.h>

inline void delay( unsigned long ms ) {
	Sleep( ms );
}

#else  /* presume POSIX */
#include <unistd.h>

inline void delay(unsigned long ms) {
	usleep(ms * 1000);
}

#endif

using namespace std;

/* LIST OF CHARACTERS to be skipped during striping HTML tags */
char AVOIDCHARS[] = { '\n', // new line
		'\t'  // tab
		};

int AVOIDCHARS_COUNT = 2;

/*
 ****************************
 *  1. CURL HELPER FUNCTIONS
 ****************************
 */

/***************************************************************
 *  writeCallback - CURL
 *  Call back function for reading URL content from curl.
 *  
 ***************************************************************/
size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up) {
	//callback must have this declaration
	//buf is a pointer to the data that curl has for us
	//size*nmemb is the size of the buffer

	for (unsigned int c = 0; c < size * nmemb; c++)
		WebSpider::curlData.push_back(buf[c]);

	return size * nmemb; //tell curl how many bytes we handled
}

/***************************************************************
 *  write_data - CURL
 *  Call back function for download image
 *  
 ***************************************************************/
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

/***************************************************************
 *  WebSpider::GetWebPage
 *  Read web page from _url path provided.
 *  
 ***************************************************************/
bool WebSpider::GetWebPage(string _url) {
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();

	// Initialize data
	WebSpider::curlData.clear();

	if (curl) {
		/* init curl options */
		curl_easy_setopt(curl, CURLOPT_URL, _url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); //tell curl to output its progress

		res = curl_easy_perform(curl);

		/* always cleanup */
		curl_easy_cleanup(curl);
		curl_global_cleanup();

		return true;
	} else {
		ostringstream trace;
		trace << "WebSpider::GetWebPage FAILED [" << _url << "]" << endl;
		trc.LogTrace(trace.str(), ERR);
		return false;
	}
}

/*
 **********************************************
 *  2. Reading & Processing HTML content
 **********************************************
 */
/***********************************************************************************
 *  WebSpider::AddSaveIMGRule
 *  
 *  Parameters :
 *     string _ruleName  (in) [Rule name]
 *     string _fileInImg (in) [text need to find in img tag]
 *  
 *  Return :
 *      bool (out) status
 *  
 *  Add Tags to list for finding content on web page
 *  
 **********************************************************************************/
bool WebSpider::AddSaveIMGRule(string _ruleName, string _findInImg) {
	if (!_ruleName.size() || !_findInImg.size()) {
		ostringstream trace;
		trace << "WebSpider::AddSaveIMGRule Invalid rule passed [" << _ruleName
				<< "][" << _findInImg << "]" << endl;
		trc.LogTrace(trace.str(), ERR);
		return false;
	}

	IMGDownloadTag d;
	d.ruleName = _ruleName;
	d.findInImg = _findInImg;

	saveImgRule.push_back(d);

	ostringstream trace;
	trace
			<< "WebSpider::AddSaveIMGRule -> One Rule to save Image is added with name [ "
			<< _ruleName << " ] " << endl;
	trc.LogTrace(trace.str(), INFO);

	return true;

}
/***********************************************************************************
 *  WebSpider::AddURLToVisitedList
 *
 *  Parameters :
 *     string url  (in) [Visited/To Ignore URL while processing]
 *
 *  Return :
 *      bool (out) status
 *
 * Add URLs to visited/To Ignore URL while processing
 *
 **********************************************************************************/
bool WebSpider::AddURLToVisitedList(string url) {
	listProcessedURLs[url] = 1;
	return true;
}
/***********************************************************************************
 *  WebSpider::AddIgnorePatternURLs
 *
 *  Parameters :
 *     string url  (in) [Ignore pattern URL]
 *
 *  Return :
 *      bool (out) status
 *
 *  Add URL pattern to ignore list.
 *
 **********************************************************************************/
bool WebSpider::AddIgnorePatternURLs(string url) {
	// Current implementation only ignores URL which maps to base urls
	// http://allcompanieslist.com/Gurgaon/
	// will ignore any url with
	// http://allcompanieslist.com/Gurgaon/MNC-private-limited-companies-list-database
	ignoreURLPatterns.push_back(url);
	return true;
}
/***********************************************************************************
 *  WebSpider::AddHTMLDataRule
 *  
 *  Parameters :
 *     string _ruleName    (in) [Rule name]
 *     string _fullHTMLTag (in) [Full tag to be found in html data]
 *     bool mustHave       (in) [if set to false value is optional]  
 * 
 *  Return :
 *      bool (out) status
 *  
 *  Add Tags to list for finding content on web page
 *  
 **********************************************************************************/
bool WebSpider::AddHTMLDataRule(string _ruleName, string _fullHTMLTag,
		bool mustHave, int size) {
	if (!_ruleName.size() || !_fullHTMLTag.size()) {
		ostringstream trace;
		trace << "WebSpider::AddTags Invalid rule passed [" << _ruleName << "]["
				<< _fullHTMLTag << "]" << endl;
		trc.LogTrace(trace.str(), ERR);
		return false;
	}

	string baseTag = GetBaseTag(_fullHTMLTag);

	HtmlDataTag d;
	d.baseTag = baseTag;
	d.fullHTMLtag = _fullHTMLTag;
	d.ruleName = _ruleName;
	d.must = mustHave;
	d.FieldSize = size;

	saveHTMLRule.push_back(d);

	ostringstream trace;
	trace << "WebSpider::AddTags -> One HTML Read Data rule added with name [ "
			<< _ruleName << " ] " << endl;
	trc.LogTrace(trace.str(), INFO);

	return true;
}
/***********************************************************************************
 *  WebSpider::AddHTMLDataRule
 *
 *  Parameters :
 *     string _ruleName    (in) [Rule name]
 *     string _fullHTMLTag (in) [Full tag to be found in html data]
 *     bool mustHave       (in) [if set to false value is optional]
 *
 *  Return :
 *      bool (out) status
 *
 *  Add Tags to list for finding content on web page
 *
 **********************************************************************************/
bool WebSpider::AddSubHTMLDataRule(HtmlDataTag _BaseHTMLTag,
		vector<SubHTMLDataTag> &subHTML) {
	if (subHTML.size() == 0) {
		ostringstream trace;
		trace << "WebSpider::AddSubHTMLDataRule Invalid rule passed " << endl;
		trc.LogTrace(trace.str(), ERR);
		return false;
	}

	vector<SubHTMLDataTag> subRules;
	bool mustHaveFlag = false;

	for (unsigned int i = 0; i < subHTML.size(); ++i) {
		if (subHTML[i].ruleName.empty() || subHTML[i].ruleHTMLTag.empty()
				|| subHTML[i].dataHTMLTag.empty()) {
			continue;
		}
		SubHTMLDataTag tmp;

		tmp.dataHTMLTag = subHTML[i].dataHTMLTag;
		tmp.dataBaseTag = GetBaseTag(subHTML[i].dataHTMLTag);
		tmp.mustHave = subHTML[i].mustHave;
		tmp.ruleName = subHTML[i].ruleName;
		tmp.ruleHTMLTag = subHTML[i].ruleHTMLTag;

		if (!mustHaveFlag && tmp.mustHave)
			mustHaveFlag = true;

		subRules.push_back(tmp);
	}

	CommonHTMLDataTag toPush;

	// Updating baseTag from full HTML Tag
	_BaseHTMLTag.baseTag = GetBaseTag(_BaseHTMLTag.fullHTMLtag);

	toPush.subRules = subRules;
	toPush.baseTag = _BaseHTMLTag;
	toPush.mustHave = mustHaveFlag;

	saveMultiHTMLRules.push_back(toPush);

	return true;
}
/***********************************************************************************
 *  WebSpider::ReplaceHTMLTagWith
 *  
 *  Parameters :
 *     string _htmltag    (in) [Rule name]
 *     string _replacewith (in) [Full tag to be found in html data]
 * 
 *  Return :
 *      Nothing
 *  
 *  Specify rules for data retrieved from HTML data to Replace specific tag with
 *  user defined data.
 *  
 **********************************************************************************/
void WebSpider::ReplaceHTMLTagWith(string _htmltag, string _replacewith,
		string _forHTMLRule) {
	if (!_htmltag.size() || !_replacewith.size() || !_forHTMLRule.size()) {
		ostringstream trace;
		trace << "WebSpider::ReplaceHTMLTagWith Invalid rule passed ["
				<< _htmltag << "][" << _replacewith << "][" << _forHTMLRule
				<< "]" << endl;
		trc.LogTrace(trace.str(), ERR);
		return;
	}

	ReplaceHTMLTag d;
	d.htmlTag = _htmltag;
	d.replaceStr = _replacewith;
	d.forHTMLRule = _forHTMLRule;

	replaceHTMLStrList.push_back(d);
}
/***********************************************************************************
 *  WebSpider::ReplaceHTMLTagWith
 *  
 *  Parameters :
 *     string _data   (in) [HTML data]
 *
 *  Return :
 *    string (out) [new data]
 *  
 *  Replace specified tags/data from content recevied.
 *
 * WebSpider::RunReplaceHTMLRules_SingleRule - Helper function to run for single rule
 * Returns final answer.
 *  
 **********************************************************************************/
string WebSpider::RunReplaceHTMLRules(string &_data, string &htmlRuleName) {
	string tData = _data;
	string ans;
	bool replaced = false;
	for (int i = 0; i < replaceHTMLStrList.size(); ++i) {
		if (replaceHTMLStrList[i].forHTMLRule == htmlRuleName) {
			replaced = true;
			ans.clear();
			ans = RunReplaceHTMLRules_SingleRule(i, tData);
			tData = ans;
		}
	}

	if (!replaced) {
		ostringstream trace;
		trace
				<< "WebSpider::RunReplaceHTMLRules : No Rule defined for htmlRuleName ["
				<< htmlRuleName << "]" << endl;
		trc.LogTrace(trace.str(), DBG);
		return tData;
	}
	return ans;
}
string WebSpider::RunReplaceHTMLRules_SingleRule(int i, string &tData) {
	string ans;
	size_t pPos = 0;
	size_t pos = tData.find(replaceHTMLStrList[i].htmlTag);
	while (pos != string::npos) {

		for (int j = pPos; j < pos; ++j)
			ans.push_back(tData[j]);

		for (int j = pos, k = 0;
				j < pos + replaceHTMLStrList[i].replaceStr.length(); ++j)
			ans.push_back(replaceHTMLStrList[i].replaceStr[k]);

		pPos = pos + replaceHTMLStrList[i].htmlTag.length();
		pos = tData.find(replaceHTMLStrList[i].htmlTag, pos + 1);
	}

	if (pPos < tData.length() - 1) {
		for (int j = pPos; j < tData.length() - 1; ++j)
			ans.push_back(tData[j]);
	}
	return ans;
}
/***********************************************************************************
 *  WebSpider::GetHTMLDataForTag
 *  
 *  Parameters :
 *  
 *      string htmlTag     (in)  [html tag < .. > to be found in URL html code]
 *      string tag         (in)  [type of tag e.g "div,p,tr"]
 *      string data        (in)  [URL data read from currentURL]
 *      vector<string> ans (out) [Content for html tag fulltag]
 * 
 *  Return :
 *      bool (out) status
 *  
 *  Get HTML data from HTML tags
 *  Note : tag passed in function should be pair tags. (rite :P)
 *  Note : HTML tags are not striped after using call html strip if required.
 *  
 **********************************************************************************/
bool WebSpider::GetHTMLDataForTag(string &htmlTag, string &tag,
		const string &data, vector<string> &ans) {
	if (!tag.size() || !data.size() || !htmlTag.size()) {
		ostringstream trace;
		trace
				<< "WebSpider::GetHTMLDataForTag ERROR invalid length htmlTag.size() "
				<< htmlTag.size() << "tags.size() " << tag.size()
				<< " : data.size() " << data.size() << endl;
		trc.LogTrace(trace.str(), ERR);
		return false;
	}

	// Read each tags one by one
	size_t pos = data.find(htmlTag);
	string tmp;

	while (pos != string::npos) {
		tmp = GetHTMLDataForTagPos(tag, data, pos);

		if (tmp.size())
			ans.push_back(tmp);

		pos = data.find(htmlTag, pos + 1);
	}

	// Log a info message if tag not found in current URL
	if (!ans.size()) {
		ostringstream trace;
		trace << "WebSpider::GetHTMLDataForTag tag " << htmlTag
				<< "not found in URL " << currentURL << endl;
		trc.LogTrace(trace.str(), INFO);

		// as no data found
		return false;
	}

	// data found
	return true;
}
/*********************************************************************************
 *  WebSpider::GetHTMLDataForTagPos
 *  
 *  Parameters :
 *  
 *      string tag         (in)  [type of tag e.g "div,p,tr"]
 *      string data        (in)  [URL data read from currentURL]
 *      size_t pos         (in)  [position in data from start reading]
 * 
 *  Return :
 *      string (out) required data
 *  
 *  Helper function for GetHTMLDataForTag to get data for tag at given position.
 *  
 **********************************************************************************/
string WebSpider::GetHTMLDataForTagPos(string &tag, const string &data,
		size_t pos) {
	string ans;
	string baseTag = GetBaseTag(tag);
	string closingTag = GetClosingTag(tag);
	size_t closingTagPos = 0;

	int i;

	if (!closingTag.size()) {
		ostringstream trace;
		trace
				<< "WebSpider::GetHTMLDataForTagPos Closing tag not known for tag :"
				<< tag << endl;
		trc.LogTrace(trace.str(), ERR);
		return ans;
	}

	closingTagPos = data.find(closingTag, pos + 1);

	// Reach to first closing '>'
	for (i = pos; data[i] != '>'; ++i)
		;

	// move 1 more step
	++i;

	while (i != closingTagPos) {
		ans.push_back(data[i]);

		if (data[i] == '<') // html tag encountered
				{
			string subTag = GetCompleteTagAtPosition(data, i);
			string subBaseTag = GetBaseTag(subTag);

			if (subBaseTag == baseTag) {
				// Find next
				closingTagPos = data.find(closingTag, closingTagPos + 1);
			}
		}
		++i;
	}

	return ans;
}
/*********************************************************************************
 *  WebSpider::GetCompleteTagAtPosition
 *  
 *  Parameters :
 *    string data (in) [html data]
 *    int pos     (in) [position of starting html tag]
 * 
 *  Return :
 *      string (out) corresponding full tag at given postion
 *  
 *  Helper function for GetHTMLDataForTag to get full html tag at given position
 *  
 **********************************************************************************/
string WebSpider::GetCompleteTagAtPosition(const string &data, int pos) {
	string ans;
	ostringstream trace;

	if (data[pos] != '<') {
		// Err
		trace << "WebSpider::GetCompleteTagAtPosition Invalid starting pos : "
				<< pos << endl;
		trc.LogTrace(trace.str(), ERR);
		return ans;
	}

	for (int i = pos; data[i] != '>'; ++i)
		ans.push_back(data[i]);
	ans.push_back('>');

	trace << "WebSpider::GetCompleteTagAtPosition returns [" << ans << "]"
			<< endl;
	trc.LogTrace(trace.str(), DBG);

	return ans;
}

/*********************************************************************************
 *  WebSpider::GetClosingTag
 *  
 *  Parameters :
 *    string tag (in) [type of tag e.g "div,p,tr"]
 * 
 *  Return :
 *      string (out) corresponding closing tag
 *  
 *  Helper function for GetHTMLDataForTag to get Base tag for supplied html tag
 *  
 **********************************************************************************/
string WebSpider::GetBaseTag(string &tag) {
	string ans;
	string buf;
	buf.push_back('<');

	if (!tag.size() || tag.at(0) != '<') {
		ostringstream trace;
		trace << "WebSpider::GetBaseTag Invalid size or invalid tag : " << tag
				<< endl;
		trc.LogTrace(trace.str(), ERR);
		return ans;
	}

	for (int i = 1; i < tag.size(); ++i) {
		if (tag[i] == ' ' && buf.size() > 1 || tag[i] == '>')
			break;
		buf.push_back(tag[i]);
	}

	buf.push_back('>');

	return buf;
}
/*********************************************************************************
 *  WebSpider::GetClosingTag
 *  
 *  Parameters :
 *    string tag (in) [type of tag e.g "div,p,tr"]
 * 
 *  Return :
 *      string (out) corresponding closing tag
 *  
 *  Helper function for GetHTMLDataForTag to get closing tag for opening tag
 *  
 **********************************************************************************/
string WebSpider::GetClosingTag(string &tag) {
	string ans;
	string buf = GetBaseTag(tag);

	if (!strcmp(buf.c_str(), "<div>"))
		ans.assign("</div>");
	else if (!strcmp(buf.c_str(), "<span>"))
		ans.assign("</span>");
	else if (!strcmp(buf.c_str(), "<body>"))
		ans.assign("</body>");
	else if (!strcmp(buf.c_str(), "<head>"))
		ans.assign("</head>");
	else if (!strcmp(buf.c_str(), "<html>"))
		ans.assign("</html>");
	else if (!strcmp(buf.c_str(), "<title>"))
		ans.assign("</title>");
	else if (!strcmp(buf.c_str(), "<p>"))
		ans.assign("</p>");
	else if (!strcmp(buf.c_str(), "<h1>"))
		ans.assign("</h1>");
	else if (!strcmp(buf.c_str(), "<h2>"))
		ans.assign("</h2>");
	else if (!strcmp(buf.c_str(), "<pre>"))
		ans.assign("</pre>");
	else if (!strcmp(buf.c_str(), "<blockquote>"))
		ans.assign("</blockquote>");
	else if (!strcmp(buf.c_str(), "<i>"))
		ans.assign("</i>");
	else if (!strcmp(buf.c_str(), "<b>"))
		ans.assign("</b>");
	else if (!strcmp(buf.c_str(), "<tt>"))
		ans.assign("</tt>");
	else if (!strcmp(buf.c_str(), "<em>"))
		ans.assign("</em>");
	else if (!strcmp(buf.c_str(), "<strong>"))
		ans.assign("</strong>");
	else if (!strcmp(buf.c_str(), "<cite>"))
		ans.assign("</cite>");
	else if (!strcmp(buf.c_str(), "<ol>"))
		ans.assign("</ol>");
	else if (!strcmp(buf.c_str(), "<ul>"))
		ans.assign("</ul>");
	else if (!strcmp(buf.c_str(), "<li>"))
		ans.assign("</li>");
	else if (!strcmp(buf.c_str(), "<a>"))
		ans.assign("</a>");
	else if (!strcmp(buf.c_str(), "<table>"))
		ans.assign("</table>");
	else if (!strcmp(buf.c_str(), "<tr>"))
		ans.assign("</tr>");
	else if (!strcmp(buf.c_str(), "<td>"))
		ans.assign("</td>");
	else {
		ostringstream trace;
		trace << "WebSpider::GetClosingTag Closing tag not in the list : "
				<< tag << " buf : " << buf << endl;
		trc.LogTrace(trace.str(), ERR);
	}

	ostringstream trace;
	trace << "WebSpider::GetClosingTag Tag : " << tag << " Buf : " << buf
			<< " Closing tag : " << ans << endl;
	trc.LogTrace(trace.str(), DBG);

	return ans;
}

/***************************************************************
 *  WebSpider::SetEscapeSequense
 *  
 *  Parameters :
 *      string input (in)
 *  
 *  Return :
 *      string (out) 
 *  
 *  To put escape sequense before running query
 *  
 ***************************************************************/
string WebSpider::SetEscapeSequense(string input) {

	string ans;
	if (!db) {
		ostringstream trace;
		trace
				<< "WebSpider::SetEscapeSequense : ERROR :database connection is not present."
				<< endl;
		trc.LogTrace(trace.str(), ERR);
		return ans;
	}

	for (int i = 0; i < input.size(); ++i) {
		if (db->iSsingleQuotesQuery()) {
			if (input[i] == '\'') {
				ans.push_back('\\');
				ans.push_back('\'');
			} else {
				ans.push_back(input[i]);
			}
		} else {
			if (input[i] == '"') {
				ans.push_back('\\');
				ans.push_back('"');
			} else {
				ans.push_back(input[i]);
			}
		}
	}
	return ans;
}

/***************************************************************
 *  WebSpider::GetOutBoundLinks
 *  
 *  Parameters :
 *      vector<string> &_outBound (out) [list of out bound urls]
 *      string &data (in) [URL data read]
 *  
 *  Return :
 *      Nothing
 *  
 *  Get all outbound URLs from given HTML data.
 *  
 ***************************************************************/
void WebSpider::GetOutBoundLinks(vector<string> &_outBound, string &data) {
	size_t pos = data.find("a href", 0);
	string href;
	while (pos != string::npos) {
		href = GetURLFromAHref(pos, data);
		if (href[0] == '/') {
			// domain need to be supplied
			href = baseWebsiteURL + href;
		}
		if (CheckAndValidateURL(href))
			_outBound.push_back(href);
		pos = data.find("a href", pos + 1);
	}
}

/***************************************************************
 *  WebSpider::GetURLFromAHref
 *  
 *  Parameters :
 *      site_t pos  [position in data from when we found "a href"]
 *      string &data (in) [URL data read]
 *  
 *  Return :
 *      string (out) [URL read from "a href" tag]
 *  
 *  Get Out bound urls from "a href" tag.
 *  
 ***************************************************************/
string WebSpider::GetURLFromAHref(size_t pos, string &data) {
	string res;
	int i = pos;

	bool FoundDouble = false;
	bool FoundSingle = false;

	for (; data.size() > i; ++i) {
		if (data.at(i) == '"') {
			FoundDouble = true;
			break;
		}
		if (data.at(i) == '\'') {
			FoundSingle = true;
			break;
		}
	}

	if (FoundDouble) {
		for (int k = i + 1; data.size() > k && data.at(k) != '"'; ++k)
			res.push_back(data.at(k));
	}

	else if (FoundSingle) {
		for (int k = i + 1; data.size() > k && data.at(k) != '\''; ++k)
			res.push_back(data.at(k));
	}

	else {
		ostringstream trace;
		trace << "WebSpider::GetURLFromAHref : Error at position [" << pos
				<< "]" << endl;
		trc.LogTrace(trace.str(), ERR);
		//assert(0);
		return res;
	}

	return res;
}

/***************************************************************
 *  WebSpider::Run
 *  - Running WebCrawler - 
 *  
 *  Parameters :
 *          Nothing
 *  Return :
 *          bool (out) status
 *  
 *  Vars usage in logic:
 *  Q :
 *       holds number of URLs in queue need to be processed.
 *  
 *  listProcessedURLs :
 *       holds URL status comes in queue for once, to avoid
 *       processing same URL again.
 *                        
 ***************************************************************/
bool WebSpider::Run() {
	string insertStatTemplete = db->GetInsertStatement(saveHTMLRule,
			saveImgRule, saveMultiHTMLRules);
	string statement;
	bool executeQuery = true;
	int numOfQuery = 0;

	// If we are resuming Crawling no need to do re-init.
	if (Q.empty()) {
		// One time task in init only
		if (!db->CheckTableName()) {
			ostringstream trace;
			trace << "WebSpider::Run - DB Table name is not set." << endl;
			trc.LogTrace(trace.str(), ERR);

			cout
					<< "DB should be configured, correctly before running Crawler.\n";
			return false;
		}

		if (!db->CreateTable(saveHTMLRule, saveImgRule, saveMultiHTMLRules)) {
			cout << "Table creation failed\n";
			if (db->DropTable(db->GetTableName())
					&& db->CreateTable(saveHTMLRule, saveImgRule,
							saveMultiHTMLRules))
				cout << "Dropped Table and created again\n";
			else
				return false;
		} else
			cout << "Table Created\n";

		// Initialize Queue
		Q.push(baseURL);
		listProcessedURLs[baseURL] = 1;
	} else {
		// Find number of records in the table
		imgCount = db->NumberOfRecords(db->GetTableName());
	}

	// Check if Image Download path is set before running
	if (saveImgRule.size() && !saveImagePath.size()) {
		ostringstream trace;
		trace
				<< "WebSpider::Run - Set Image Download Path before Running - Exiting"
				<< endl;
		trc.LogTrace(trace.str(), ERR);
		return false;
	}

	// Process Queue
	while (!Q.empty()) {
		currentURL = Q.front();

		// Add delay if required
		if (getDelayBtwEachCall() > 0) {
			delay(getDelayBtwEachCall());
		}

		if (showStatus) {
			cout << trc.GetCurrentTime()  << " " << currentURL << endl;
			cout.flush();
		}

		Q.pop();

		if (!GetWebPage(currentURL)) {
			ostringstream trace;
			trace << "WebSpider::Run (" << currentURL
					<< ") [There were error during Processing]" << endl;
			trc.LogTrace(trace.str(), ERR);
			return false;
		}

		executeQuery = true;
		statement.clear();

		// Adding URL to MYSQL statement
		statement = insertStatTemplete;

		if (db->iSsingleQuotesQuery())
			statement += "E'" + SetEscapeSequense(currentURL) + "'";
		else
			statement += "\"" + SetEscapeSequense(currentURL) + "\"";

		// Process HTML Rules
		if (saveHTMLRule.size()) {
			bool res;
			for (int i = 0; i < saveHTMLRule.size(); ++i) {
				vector<string> ans;
				bool res = GetHTMLDataForTag(saveHTMLRule[i].fullHTMLtag,
						saveHTMLRule[i].baseTag, WebSpider::curlData, ans);
				if (!res) {
					if (saveHTMLRule[i].must) {
						ostringstream trace;
						trace << "WebSpider::Run (" << currentURL
								<< ") [Rule : " << saveHTMLRule[i].ruleName
								<< "] Failed. Not executing other rules"
								<< endl;
						trc.LogTrace(trace.str(), ERR);
						executeQuery = false;
						break;
					} else {
						if (db->iSsingleQuotesQuery())
							statement += ", ''";
						else
							statement += ", \"\"";
					}
				} else {
					if (ans.size() > 1) {
						ostringstream trace;
						trace << "WebSpider::Run (" << currentURL
								<< ") [Rule : " << saveHTMLRule[i].ruleName
								<< "] matches more than once, only logging 1st occurrance."
								<< endl;
						trc.LogTrace(trace.str(), DBG);
					}
					/* Check if any replace string role is defined */
					string f = RunReplaceHTMLRules(ans[0],
							saveHTMLRule[i].ruleName);
					string text = StripHTML(f, true);

					if (db->iSsingleQuotesQuery())
						statement += ", E'" + SetEscapeSequense(text) + "'";
					else
						statement += ", \"" + SetEscapeSequense(text) + "\"";
				}
			}
		}

		// Read content from page as per sub HTML Rules
		if (saveMultiHTMLRules.size() && executeQuery) {
			for (unsigned int i = 0; i < saveMultiHTMLRules.size(); ++i) {
				vector<string> ans;
				bool res = GetHTMLDataForTag(
						saveMultiHTMLRules[i].baseTag.fullHTMLtag,
						saveMultiHTMLRules[i].baseTag.baseTag,
						WebSpider::curlData, ans);
				if (res) {
					// To keep track of processed ans tags.
					vector<bool> bAns;
					bAns.resize(ans.size());
					for (unsigned int j = 0;
							j < saveMultiHTMLRules[i].subRules.size(); ++j) {
						vector<string> dataAns;
						for (unsigned int k = 0; k < ans.size(); ++k) {
							if (bAns[k]) 					// Already processed
								continue;
							if (ans[k].find(
									saveMultiHTMLRules[i].subRules[j].ruleHTMLTag)
									!= string::npos) {
								GetHTMLDataForTag(
										saveMultiHTMLRules[i].subRules[j].dataHTMLTag,
										saveMultiHTMLRules[i].subRules[j].dataBaseTag,
										ans[k], dataAns);
								bAns[k] = true;
							}
						}
						if (dataAns.size() && !dataAns[0].empty()) {
							if (db->iSsingleQuotesQuery())
								statement += ", E'"
										+ SetEscapeSequense(dataAns[0]) + "'";
							else
								statement += ", \""
										+ SetEscapeSequense(dataAns[0]) + "\"";
						} else {
							if (saveMultiHTMLRules[i].subRules[j].mustHave) {
								executeQuery = false;
								break;
							} else {
								if (db->iSsingleQuotesQuery())
									statement += ", ''";
								else
									statement += ", \"\"";
							}
						}
					}
					if (!executeQuery) // If any must have rule is failed no need to push data to DB
						break;
				} else {
					executeQuery = false;
				}
				// If current saveMultiHTML rule is must have ignore futher execution.
				if (!executeQuery && saveMultiHTMLRules[i].mustHave) {
					ostringstream trace;
					trace << "WebSpider::Run (" << currentURL << ") [Rule : "
							<< saveMultiHTMLRules[i].baseTag.ruleName
							<< "] matches more than once, only logging 1st occurrance."
							<< endl;
					trc.LogTrace(trace.str(), ERR);
					executeQuery = false;
					break;
				}
			}
		}

		// Processing Images Download
		if (saveImgRule.size() && executeQuery) {
			for (int i = 0; i < saveImgRule.size(); ++i) {
				string imgPath = GetImagePathForRule(saveImgRule[i],
						WebSpider::curlData);

				if (!imgPath.size()) {
					ostringstream trace;
					trace << "WebSpider::Run (" << currentURL << ") For Rule "
							<< saveImgRule[i].ruleName << " Image not found "
							<< endl;
					trc.LogTrace(trace.str(), ERR);
					executeQuery = false;
					break;
				} else {
					// Save Image at location
					string imgName = GetRandomImageName(imgPath);
					bool res = DownloadImage(imgPath, saveImagePath, imgName);
					// Prepare statement
					if (db->iSsingleQuotesQuery())
						statement += ", '" + saveImagePath + imgName + "'";
					else
						statement += ", \"" + saveImagePath + imgName + "\"";
				}
			}
		}

		statement += ")";

		{
			ostringstream trace;
			trace << "WebSpider::Run - Query :" << statement << endl;
			trc.LogTrace(trace.str(), DBG);
		}

		// Execute Statement
		if (executeQuery) {
			++numOfQuery;
			if (!db->ExecuteQuery(statement)) {
				ostringstream trace;
				trace << "WebSpider::Run Execution Query failed: " << statement;
				trc.LogTrace(trace.str(), ERR);
				return false;
			}

			if (limit > 0 && limit == numOfQuery) {
				ostringstream trace;
				trace
						<< "WebSpider::Run Limit of processed URL reached, exiting  limit : "
						<< limit << " : number of Query " << numOfQuery << endl;
				trc.LogTrace(trace.str(), DBG);
				return true;
			}
		}

		// Update number of Processed URLs
		++processedURLs;

		// Log visited URL
		LogURL(currentURL);

		// Capture Out Bound URLs
		vector<string> outBound;
		outBound.clear();
		GetOutBoundLinks(outBound, WebSpider::curlData);

		for (unsigned int i = 0; i < outBound.size(); ++i) {
			if (listProcessedURLs[outBound[i]] == 0) {
				listProcessedURLs[outBound[i]] = 1;
				Q.push(outBound[i]);
			}
		}
	}

	/* Completed Successfully */
	return true;
}

/******************************************************************
 *  WebSpider::RunSinglePage
 *  - Running WebCrawler for single page - 
 *  
 *  Parameters :
 *          string _rul (in)
 *  Return :
 *          bool (out) status
 *  
 *  To Test all conditions set during configuration runs perfectly.
 *                        
 ******************************************************************/
bool WebSpider::RunSinglePage(string _url) {

	string insertStatTemplete = db->GetInsertStatement(saveHTMLRule,
			saveImgRule, saveMultiHTMLRules);
	string statement = insertStatTemplete;
	bool executeQuery = true;

	if (!db->ConnectToServer()) {
		cout << "DB Server Connection failed\n";
		return false;
	} else
		cout << "DB Server connected\n";

	if (!db->CreateTable(saveHTMLRule, saveImgRule, saveMultiHTMLRules)) {
		cout << "DB Table creation failed\n";

		if (isForceCreateTable()) {
			cout << "DB Dropping table and creating again\n";
			db->DropTable(db->GetTableName());
			if (!db->CreateTable(saveHTMLRule, saveImgRule,
					saveMultiHTMLRules)) {
				cout << "DB Table creation failed Again\n";
				return false;
			} else
				cout << "DB Table Created with Retry, way to go\n";
		} else {
			return false;
		}
	} else
		cout << "DB Table Created, way to go\n";

	// Add delay if required
	if (getDelayBtwEachCall() > 0) {
		cout << "Delaying....." << trc.GetCurrentTime() << endl;
		delay(getDelayBtwEachCall());
		cout << "Delayed....." << trc.GetCurrentTime() << endl;
	}

	if (!GetWebPage(_url)) {
		ostringstream trace;
		trace << "WebSpider::RunSinglePage (" << _url
				<< ") [There were error during Processing]" << endl;
		trc.LogTrace(trace.str(), ERR);
		return false;
	}

	// OutBound URLs list
	vector<string> outBound;
	outBound.clear();
	cout << "OutBound URLs" << endl;
	GetOutBoundLinks(outBound, WebSpider::curlData);
	for (unsigned int i = 0; i < outBound.size(); ++i) {
		cout << outBound[i] << endl;
	}

	// Log visited URL
	LogURL(currentURL);

	if (db->iSsingleQuotesQuery())
		statement += "'" + _url + "'";
	else
		statement += "\"" + _url + "\"";

	/* Read content from page as per HTML Rules */
	if (saveHTMLRule.size()) {
		for (int i = 0; i < saveHTMLRule.size(); ++i) {
			vector<string> ans;
			bool res = GetHTMLDataForTag(saveHTMLRule[i].fullHTMLtag,
					saveHTMLRule[i].baseTag, WebSpider::curlData, ans);
			if (res) {
				/* Check if any replace string role is defined */
				string f = RunReplaceHTMLRules(ans[0],
						saveHTMLRule[i].ruleName);
				string text = StripHTML(f, true);
				if (db->iSsingleQuotesQuery())
					statement += ", '" + text + "'";
				else
					statement += ", \"" + text + "\"";
				cout << text << endl;
			} else {
				cout << "ERROR IN READING" << endl;
				if (saveHTMLRule[i].must) {
					executeQuery = false;
					break;
				} else {
					if (db->iSsingleQuotesQuery())
						statement += ", ''";
					else
						statement += ", \"\"";
				}
			}
		}
	}

	/* Read content from page as per sub HTML Rules */
	if (saveMultiHTMLRules.size()) {
		for (unsigned int i = 0; i < saveMultiHTMLRules.size(); ++i) {

			vector<string> ans;
			bool res = GetHTMLDataForTag(
					saveMultiHTMLRules[i].baseTag.fullHTMLtag,
					saveMultiHTMLRules[i].baseTag.baseTag, WebSpider::curlData,
					ans);
			if (res) {

				// To keep trank of processed ans tags.
				vector<bool> bAns;
				bAns.resize(ans.size());
				for (unsigned int j = 0;
						j < saveMultiHTMLRules[i].subRules.size(); ++j) {
					vector<string> dataAns;
					for (unsigned int k = 0; k < ans.size(); ++k) {

						// Already processed
						if (bAns[k])
							continue;

						// Check if current retrieved data have sub rule
						if (ans[k].find(
								saveMultiHTMLRules[i].subRules[j].ruleHTMLTag)
								!= string::npos) {
							GetHTMLDataForTag(
									saveMultiHTMLRules[i].subRules[j].dataHTMLTag,
									saveMultiHTMLRules[i].subRules[j].dataBaseTag,
									ans[k], dataAns);
							bAns[k] = true;
						}
					}
					if (dataAns.size() && !dataAns[0].empty()) {
						if (db->iSsingleQuotesQuery())
							statement += ", '" + dataAns[0] + "'";
						else
							statement += ", \"" + dataAns[0] + "\"";
						cout << dataAns[0] << endl;
					} else {
						if (saveMultiHTMLRules[i].subRules[j].mustHave) {
							executeQuery = false;
							break;
						} else {
							if (db->iSsingleQuotesQuery())
								statement += ", ''";
							else
								statement += ", \"\"";
						}
					}
				}
				// If any rule is failed no need to push data to DB
				if (!executeQuery)
					break;
			}
		}
	}

	/* Download Images if rule set */
	if (saveImgRule.size()) {
// Get Image src path
		string imgPath = GetImagePathForRule(saveImgRule[0],
				WebSpider::curlData);
		cout << " IMAGE PATH " << imgPath << endl;
		bool res = DownloadImage(imgPath, "/home/goldie/workspace/Crawler",
				"testing.jpg");
		if (res) {
			string saveImagePath("/home/goldie/workspace/Crawler/testing.jpg");
			if (db->iSsingleQuotesQuery())
				statement += ", '" + saveImagePath + "')";
			else
				statement += ", \"" + saveImagePath + "\")";
			cout << "File Downloaded " << endl;
		} else
			cout << "Error in downloading " << endl;
	}

	statement += ")";

	// One time task, in init only
	cout << statement << endl;
	cout
			<< (db->ExecuteQuery(statement) == true ?
					"MySQL : Execution done\n" : "MySQL :Execution failed\n");

	/* Dump */
	DumpStatus();

	/* Completed Successfully */
	return true;
}

/***************************************************************
 *  WebSpider::CheckAndValidateURL
 *  To validate URL
 *
 *  Parameters :
 *          string & url (in) URL
 *  Return :
 *          bool (out) status
 * 
 ***************************************************************/
bool WebSpider::CheckAndValidateURL(string &url) {
	if (url.size() < 4) {
		ostringstream trace;
		trace << "WebSpider::CheckAndValidateURL " << url
				<< " [Should have atleast 4 chars]" << endl;
		trc.LogTrace(trace.str(), DBG);
		return false;
	}
	if (url.at(0) == 'h' || url.at(0) == 'H') {
// Nothing to do
	} else {
		ostringstream trace;
		trace << "WebSpider::CheckAndValidateURL " << url
				<< " [Should start with http or HTTP]" << endl;
		trc.LogTrace(trace.str(), DBG);
		return false;
	}

	for (unsigned int i = 0; i < ignoreURLPatterns.size(); ++i) {
		if (url.find(ignoreURLPatterns[i]) != string::npos) {
			ostringstream trace;
			trace << "WebSpider::CheckAndValidateURL " << url
					<< " [Ignore pattern URL found]" << ignoreURLPatterns[i]
					<< endl;
			trc.LogTrace(trace.str(), DBG);
			return false;
		}
	}

	if (listProcessedURLs[url] == 1) {
		ostringstream trace;
		trace << "WebSpider::CheckAndValidateURL " << url
				<< " [Should not be in visited URLs]" << endl;
		trc.LogTrace(trace.str(), DBG);
		return false;
	}

	for (int i = 0; baseWebsiteURL.size() > i && url.size() > i; ++i) {
		if (url.at(i) != baseWebsiteURL.at(i)) {
			ostringstream trace;
			trace << "WebSpider::CheckAndValidateURL " << url
					<< " [Should not have other domain ]" << endl;
			trc.LogTrace(trace.str(), DBG);
			return false;
		}
	}

	return true;
}

/***************************************************************
 *  WebSpider::CheckIfCodeNotCommentedOut
 *  To Check if given postion in HTML data is in dead code
 *  
 *  Parameters :
 *          string pos  (in) [position in HTML]
 *          string data (in) [HTML data]
 *  Return :
 *          bool (out) status
 *          true - if dead code
 *          false - if code in use
 *                        
 ***************************************************************/
bool WebSpider::CheckIfCodeNotCommentedOut(int pos, string &data) {
	size_t commentEndPos = data.find("-->", pos + 1);
	size_t commentStartPos = data.find("<!--", pos + 1);

	if (commentEndPos == string::npos) {
		ostringstream trace;
		trace
				<< "WebSpider::CheckIfCodeNotCommentedOut : Code not in dead zone cond:1"
				<< endl;
		trc.LogTrace(trace.str(), DBG);
		return false;
	}

	if (commentStartPos == string::npos) {
		ostringstream trace;
		trace
				<< "WebSpider::CheckIfCodeNotCommentedOut : Code in dead zone cond:2"
				<< endl;
		trc.LogTrace(trace.str(), DBG);
		return true;
	}

	if (commentEndPos < commentStartPos) {
		ostringstream trace;
		trace
				<< "WebSpider::CheckIfCodeNotCommentedOut : Code in dead zone cond:3"
				<< endl;
		trc.LogTrace(trace.str(), DBG);
		return true;
	}

	ostringstream trace;
	trace
			<< "WebSpider::CheckIfCodeNotCommentedOut : Code not in dead zone cond:4"
			<< endl;
	trc.LogTrace(trace.str(), DBG);
	return false;
}
/***************************************************************
 *  WebSpider::StripHTML
 *  To Remove HTML tags.
 *  
 *  Parameters :
 *          string data (in)
 *  Return :
 *          string (out)
 *                        
 ***************************************************************/
string WebSpider::StripHTML(string &data, bool removeOtherChar) {
	string ans;

	bool inHTMLtag = false;

	for (int i = 0; i < data.size(); ++i) {
		if (data[i] == '<') {
			inHTMLtag = true;
			continue;
		} else if (data[i] == '>') {
			inHTMLtag = false;

			// Adding space when required.
			if (ans.size() && ans[ans.size() - 1] != ' ')
				ans.push_back(' ');

			continue;
		}

		if (!inHTMLtag) {
			if (removeOtherChar) {
				if (CheckOtherChar(data[i]))
					ans.push_back(data[i]);
			} else
				ans.push_back(data[i]);
		}
	}
	return ans;
}

/***************************************************************
 *  WebSpider::CheckOtherChar
 *  To Remove HTML tags with other chars.
 *  
 *  Parameters :
 *          char t (in)
 *  Return :
 *          bool (out) add to output or not
 *                        
 ***************************************************************/
bool WebSpider::CheckOtherChar(char t) {
	for (int i = 0; i < AVOIDCHARS_COUNT; ++i) {
		if (t == AVOIDCHARS[i])
			return false;
	}
	return true;
}
/***************************************************************
 *  WebSpider::GetRandomImageName
 *  To Random image name to save in folder
 *  
 *  Parameters :
 *          Nothing
 *  Return :
 *          string (out) image name
 *                        
 ***************************************************************/
string WebSpider::GetRandomImageName(string &imgPath) {
	size_t pos = imgPath.find_last_of(".");
	char name[256];
	if (pos == string::npos) {
		ostringstream trace;
		trace << "WebSpider::GetRandomImageName : Image extension not found ["
				<< imgPath << "]" << endl;
		trc.LogTrace(trace.str(), ERR);

		sprintf(name, "%d.jpg", imgCount++);
		string iName(name);
		return iName;
	}
	sprintf(name, "%d.%s", imgCount++, imgPath.substr(pos + 1).c_str());
	string iName(name);
	return iName;
}
/***************************************************************
 *  WebSpider::DownloadImage
 *  To Download Image
 *  
 *  Parameters :
 *          string _imgpath (in) image Path
 *          string _saveto  (in) Where to save
 *  Return :
 *          bool (out) status
 *                        
 ***************************************************************/
bool WebSpider::DownloadImage(string _imgpath, string _saveto,
		string _saveFileName) {
	if (!_imgpath.size() || !_saveto.size()) {
		ostringstream trace;
		trace << "WebSpider::DownloadImage Invalid Path Image Path : "
				<< _imgpath << " Save to : " << _saveto << endl;
		trc.LogTrace(trace.str(), ERR);
		return false;
	}

	if (!_saveFileName.size()) {
// Use same file name as mentioned in path
// TODO - Get File name from path
		string getName = GetFileNameFromURL(_imgpath);
	}

	// TODO - Check if path is valid

	ostringstream trace;
	trace << "WebSpider::DownloadImage - Path Image Path : " << _imgpath
			<< " Save to : " << _saveto << " File Name (save) : "
			<< _saveFileName << endl;
	trc.LogTrace(trace.str(), DBG);
	string toPath = _saveto + _saveFileName;

	CURL *curl;
	FILE *fp;
	CURLcode res;
	curl = curl_easy_init();
	if (curl) {
		fp = fopen(toPath.c_str(), "wb");

		curl_easy_setopt(curl, CURLOPT_URL, _imgpath.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		res = curl_easy_perform(curl);

		/* always cleanup */
		curl_easy_cleanup(curl);
		fclose(fp);
	} else {
		ostringstream trace;
		trace << "WebSpider::DownloadImage - ERROR in CURL" << endl;
		trc.LogTrace(trace.str(), ERR);
		return false;
	}
	return true;
}

/***************************************************************
 *  WebSpider::GetImagePathForRule
 *  Helper function for Downloading image -
 *  To get src path of image for given rule
 *  
 *  Parameters :
 *          IMGDownloadTag &rule (in) image download rule
 *          string &data         (in) HTML data
 *  Return :
 *          string (out) source path
 *  
 *  Note : Currenlty Routine only processes first match
 ***************************************************************/
string WebSpider::GetImagePathForRule(IMGDownloadTag &rule, string &data) {
	ostringstream trace;
	trace << "WebSpider::GetImagePathForRule - Running for rule : "
			<< rule.ruleName << endl;
	trc.LogTrace(trace.str(), INFO);

	string ans;
	string completeTag;
	size_t pos = data.find(rule.findInImg);

	while (pos != string::npos) {
		if (!CheckIfCodeNotCommentedOut(pos, data)
				&& IsImageTag(pos, data, completeTag)) {
			ans = GetImageSrc(completeTag);
			break;
		}
		pos = data.find(rule.findInImg, pos + 1);
	}

	return ans;
}
/***************************************************************
 *  WebSpider::IsImageTag
 *  Helper function for Downloading image -
 *  To Check if matching condition is a img tag
 *  
 *  Parameters :
 *          int pos      (in) [position of match]
 *          string &data (in) [HTML data]
 *  
 *  string &completeTag (out) if img tag return
 *                            complete img tag
 *  Return :
 *          string (out) source path
 *  
 ***************************************************************/
bool WebSpider::IsImageTag(int pos, string &data, string &completeTag) {
	int i = 0;

	for (i = pos - 1; data[i] != '<'; --i) {
		if (data[i] == '>') {
			ostringstream trace;
			trace << "WebSpider::IsImageTag match found out side html tag : "
					<< endl;
			trc.LogTrace(trace.str(), ERR);

			return false;
		}
	}

	// Check for image tag
	if ((data[i + 1] == 'i' || data[i + 1] == 'I')
			&& (data[i + 2] == 'm' || data[i + 2] == 'M')
			&& (data[i + 3] == 'g' || data[i + 3] == 'G')) {
		for (int j = i; data[j] != '>'; ++j)
			completeTag.push_back(data[j]);

		ostringstream trace;
		trace << "WebSpider::IsImageTag - Returning Complete tag as : "
				<< completeTag << endl;
		trc.LogTrace(trace.str(), DBG);

		return true;
	}

	ostringstream trace;
	trace << "WebSpider::IsImageTag - IMG Tag not found : " << endl;
	trc.LogTrace(trace.str(), DBG);

	return false;
}
/***************************************************************
 *  WebSpider::GetImageSrc
 *  Helper function for Downloading image -
 *  To get src from complete img tag
 *  
 *  Parameters :
 *          string (in) complete img tag
 *  Return :
 *          string (out) source path
 *  
 ***************************************************************/
string WebSpider::GetImageSrc(string &completeTag) {
	string ans;
	if (!completeTag.size()) {
		ostringstream trace;
		trace << "WebSpider::GetImageSrc Invalid Img tag - No tag found "
				<< endl;
		trc.LogTrace(trace.str(), ERR);
		return ans;
	}

	size_t pos = completeTag.find("src");
	if (pos != string::npos) {
		int i = 0;
		for (i = pos + 1; completeTag[i] != '"'; ++i)
			;
		++i;
		for (; completeTag[i] != '"'; ++i)
			ans.push_back(completeTag[i]);

		ostringstream trace;
		trace << "WebSpider::GetImageSrc Returning Img path - " << ans << endl;
		trc.LogTrace(trace.str(), INFO);
		return ans;
	} else {
		ostringstream trace;
		trace << "WebSpider::GetImageSrc Invalid Img tag - No Src found "
				<< endl;
		trc.LogTrace(trace.str(), ERR);
		return ans;
	}
}

/***************************************************************
 *  WebSpider::GetFileNameFromURL
 *  To Retrive file name from URL
 *  
 *  Parameters :
 *          string _imgpath (in) URL path
 *  Return :
 *          sting (out) file name
 *                        
 ***************************************************************/
string WebSpider::GetFileNameFromURL(string _imgpath) {
	if (!_imgpath.size()) {
		ostringstream trace;
		trace << "WebSpider::GetFileNameFromURL - Invalid image path" << endl;
		trc.LogTrace(trace.str(), ERR);
	}

	string ans;
	size_t pos = _imgpath.find_last_of("/");
	if (pos == string::npos) {
		ostringstream trace;
		trace
				<< "WebSpider::GetFileNameFromURL - Error in getting proper file name."
				<< endl;
		trc.LogTrace(trace.str(), ERR);
	} else {
		ans = _imgpath.substr(pos + 1);
	}

	return ans;
}

/*
 **********************************************
 *  3. Exit & Entry Tasks
 **********************************************
 */

/***************************************************************
 *  WebSpider::DoExitTask
 *  To Clean the mess before exiting program.
 *  
 *  Parameters :
 *          Nothing
 *  Return :
 *          Nothing
 *                        
 ***************************************************************/
void WebSpider::DoExitTask() {
	cout << " WebSpider::DoExitTask" << endl;
	// 1. Dump current queue to file
	DumpQueue();

	// 2. Dump status related variables
	DumpStatus();

	// 3. Dump handlers

// 4. Close mysql connection
	delete db;
	// Terminate program

}

/***************************************************************
 *  WebSpider::Init
 *  To perform required task before starting.
 *  
 *  Parameters :
 *          Nothing
 *  Return :
 *          Nothing
 *                        
 ***************************************************************/
void WebSpider::Init() {
	UpdateVisitedURLfileName(DEF_VISITED_URL_LOG_FILE);
	db->ConnectToServer();
	/* Empty Queue -- Safer side */
	while (!Q.empty())
		Q.pop();
}

/***************************************************************
 *  WebSpider::DumpQueue
 *  To dump Queue to a file
 *  
 *  Parameters :
 *          Nothing
 *  Return :
 *          Nothing
 *
 ***************************************************************/
void WebSpider::DumpQueue() {
	ofstream dumpfile;
	dumpfile.open(QUEUE_DUMP_FILE);

	while (!Q.empty()) {
		dumpfile << Q.front() << endl;
		Q.pop();
	}
	dumpfile.close();
}

/***************************************************************
 *  WebSpider::DumpStatus
 *  To dump Processed URL to file
 *  
 *  Parameters :
 *          Nothing
 *  Return :
 *          Nothing
 *
 ***************************************************************/
void WebSpider::DumpStatus() {
	ofstream dumpfile;
	dumpfile.open(STATUS_DUMP_FILE);

	map<string, int>::iterator it;

	for (it = listProcessedURLs.begin(); it != listProcessedURLs.end(); ++it)
		dumpfile << (*it).first << endl;

	dumpfile.close();
}

/***************************************************************
 *  WebSpider::LoadQueue
 *  To Populate Queue from file
 *  
 *  Parameters :
 *          string file (in) File path to read from
 *  Return :
 *          Nothing
 *
 ***************************************************************/
void WebSpider::LoadQueue(string file) {
	ifstream dumpfile(file.c_str());

	if (!dumpfile.is_open()) {
		ostringstream trace;
		trace << "WebSpider::LoadQueue " << file << " [Error in opening file ]"
				<< endl;
		trc.LogTrace(trace.str(), ERR);
		return;
	}

	string line;
	int count = 0;

	while (getline(dumpfile, line)) {
		if (CheckAndValidateURL(line)) {
			++count;
			Q.push(line);
		}
	}

	dumpfile.close();

	if (showStatus)
		cout << "Queue Loaded : " << count << endl;

	if (remove(file.c_str()) == 0) {
		if (showStatus)
			cout << file << " removed  " << endl;

		ostringstream trace;
		trace << "WebSpider::LoadQueue " << file << " [ File deleted ]" << endl;
		trc.LogTrace(trace.str(), INFO);
	} else {
		if (showStatus)
			cout << file << " Error in removing file  " << endl;
		ostringstream trace;
		trace << "WebSpider::LoadQueue " << file << " [Error in deleting file ]"
				<< endl;
		trc.LogTrace(trace.str(), ERR);
	}
}

/***************************************************************
 *  WebSpider::LoadStatus
 *  To Populate Status from file
 *  
 *  Parameters :
 *          string file (in) File path to read from
 *  Return :
 *          Nothing
 *
 ***************************************************************/
void WebSpider::LoadStatus(string file) {
	ifstream dumpfile(file.c_str());

	if (!dumpfile.is_open()) {
		ostringstream trace;
		trace << "WebSpider::LoadStatus " << file << " [Error in opening file ]"
				<< endl;
		trc.LogTrace(trace.str(), ERR);
		return;
	}

	string line;
	int count = 0;

	while (getline(dumpfile, line)) {
		if (CheckAndValidateURL(line)) {
			++count;
			listProcessedURLs[line] = 1;
		}
	}

	if (showStatus)
		cout << "Status Loaded : " << count << endl;

	dumpfile.close();

	if (remove(file.c_str()) == 0) {
		if (showStatus)
			cout << file << " removed  " << endl;

		ostringstream trace;
		trace << "WebSpider::LoadStatus " << file << " [ File deleted ]"
				<< endl;
		trc.LogTrace(trace.str(), INFO);
	} else {
		if (showStatus)
			cout << file << " Error in removing file  " << endl;
		ostringstream trace;
		trace << "WebSpider::LoadStatus " << file
				<< " [Error in deleting file ]" << endl;
		trc.LogTrace(trace.str(), ERR);
	}
}

/*******************************************
 * Constructor/Destructor
 *******************************************/
WebSpider::WebSpider() {
	actionHandlers.clear();
	imgCount = 0;
	limit = 0;
	forceCreateTable = false;
	setRunOnSinglePage(false);
	//TODO: Use DB based on configuration

// Default is mySQL DB
	db = new MySqlConnection();
	//db = new PGSQLConnection();
}
WebSpider::WebSpider(string _URL, int _depth, int DB_TYPE) {
	UpdateBaseURL(_URL);
	UpdateDepth(_depth);
	actionHandlers.clear();
	forceCreateTable = false;
	setRunOnSinglePage(false);
	if (DB_TYPE == MYSQL_DB)
		db = new MySqlConnection();
	else
		db = new PGSQLConnection();
}
WebSpider::~WebSpider() {
	DoExitTask();
	baseURL.clear();
	actionHandlers.clear();
}

/*
 **********************************************
 *  4. DB related
 **********************************************
 */

/*
 **********************************************
 *  5. Configuration -- TODO
 **********************************************
 */
