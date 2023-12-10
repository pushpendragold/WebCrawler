#ifndef _H_IMG_DOWNLOAD_H_
#define _H_IMG_DOWNLOAD_H_

#include <vector>
#include <string>
using namespace std;


/* IMG download data tags */
class IMGDownloadTag {
public:
	string ruleName;
	string findInImg;

	const string& getFindInImg() const {
		return findInImg;
	}

	void setFindInImg(const string& findInImg) {
		this->findInImg = findInImg;
	}

	const string& getRuleName() const {
		return ruleName;
	}

	void setRuleName(const string& ruleName) {
		this->ruleName = ruleName;
	}
};

#endif
