#ifndef _H_HTML_DATA_TAG_H_
#define _H_HTML_DATA_TAG_H_

#include <vector>
#include <string>
using namespace std;

/* HTML data tags */
class HtmlDataTag {

public:
	string ruleName;
	string fullHTMLtag;
	string baseTag;
	int FieldSize;
	bool must;

	const string& getBaseTag() const {
		return baseTag;
	}

	void setBaseTag(const string& baseTag) {
		this->baseTag = baseTag;
	}

	int getFieldSize() const {
		return FieldSize;
	}

	void setFieldSize(int fieldSize) {
		FieldSize = fieldSize;
	}

	const string& getFullHtmLtag() const {
		return fullHTMLtag;
	}

	void setFullHtmLtag(const string& fullHtmLtag) {
		fullHTMLtag = fullHtmLtag;
	}

	bool isMust() const {
		return must;
	}

	void setMust(bool must) {
		this->must = must;
	}

	const string& getRuleName() const {
		return ruleName;
	}

	void setRuleName(const string& ruleName) {
		this->ruleName = ruleName;
	}
};

#endif
