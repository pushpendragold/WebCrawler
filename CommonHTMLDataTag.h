#ifndef _H_COMMON_HTML_DATA_TAG_H_
#define _H_COMMON_HTML_DATA_TAG_H_

#include <vector>
#include <string>
using namespace std;

// flag to determine if this field is required or not
// it holds complete HTML tag with value as applicable.
// This tag should always have same value, and ignored always, Value remained from rest of base tag will be stored in DB.

class CommonHTMLDataTag {
public:
	HtmlDataTag baseTag;             // Html Tag which can have multiple sub tags
	vector<SubHTMLDataTag> subRules; // On htmlTag if any of the subrule is matched it will be added to sql list;
	bool mustHave;                   // if Any of the sub HTML rule is required

	const HtmlDataTag& getBaseTag() const {
		return baseTag;
	}

	void setBaseTag(const HtmlDataTag& baseTag) {
		this->baseTag = baseTag;
	}

	const vector<SubHTMLDataTag>& getSubRules() const {
		return subRules;
	}

	void setSubRules(const vector<SubHTMLDataTag>& subRules) {
		this->subRules = subRules;
	}
};

#endif
