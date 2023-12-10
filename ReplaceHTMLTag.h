#ifndef _H_REPLACE_HTML_TAG_H_
#define _H_REPLACE_HTML_TAG_H_

#include <string>
using namespace std;


class ReplaceHTMLTag {
public:
	string htmlTag;
	string replaceStr;
	string forHTMLRule;

	const string& getForHtmlRule() const {
		return forHTMLRule;
	}

	void setForHtmlRule(const string& forHtmlRule) {
		forHTMLRule = forHtmlRule;
	}

	const string& getHtmlTag() const {
		return htmlTag;
	}

	void setHtmlTag(const string& htmlTag) {
		this->htmlTag = htmlTag;
	}

	const string& getReplaceStr() const {
		return replaceStr;
	}

	void setReplaceStr(const string& replaceStr) {
		this->replaceStr = replaceStr;
	}
};

#endif
