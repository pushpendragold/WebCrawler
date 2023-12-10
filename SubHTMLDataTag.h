#ifndef _H_SUB_HTML_DATA_TAG_H_
#define _H_SUB_HTML_DATA_TAG_H_

#include <string>
using namespace std;

/* Rule defined to find sub HTML tag within bigger HTML tag.
 * This rule will have DB field based on sub HTML "ruleName".
 * One Rule can have many sub rule names based on conditions (must have or optional).
 * */
class SubHTMLDataTag {
public:
	string ruleName; /* This will be column name in DB */
	string ruleHTMLTag;
	string dataHTMLTag;
	string dataBaseTag;
	bool mustHave; /* Data is only dump if all rules with must have fields are found */

	bool isMustHave() const {
		return mustHave;
	}

	void setMustHave(bool mustHave) {
		this->mustHave = mustHave;
	}

	const string& getRuleHtmlTag() const {
		return ruleHTMLTag;
	}

	void setRuleHtmlTag(const string& ruleHtmlTag) {
		ruleHTMLTag = ruleHtmlTag;
	}

	const string& getRuleName() const {
		return ruleName;
	}

	void setRuleName(const string& ruleName) {
		this->ruleName = ruleName;
	}

	const string& getDataBaseTag() const {
		return dataBaseTag;
	}

	void setDataBaseTag(const string& dataBaseTag) {
		this->dataBaseTag = dataBaseTag;
	}

	const string& getDataHtmlTag() const {
		return dataHTMLTag;
	}

	void setDataHtmlTag(const string& dataHtmlTag) {
		dataHTMLTag = dataHtmlTag;
	}
};

#endif
