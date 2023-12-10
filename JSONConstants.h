#ifndef _H_JSON_CONSTANS_H_
#define _H_JSON_CONSTANS_H_
#include <string>

using namespace std;

class JSONConstants {
public:
	enum {
		BASE_URL, DB, /* database */

		/****************************/
		/* DB Connection parameters */
		/****************************/
		DB_TYPE, /* DB_type */
		DB_HOST, /* DB_host */
		DB_USER, /* DB username */
		DB_PASSWORD, /* DB password */
		DB_NAME, /* DB database name */
		DB_PORT, /* DB Port address */
		DB_TABLE_NAME, /* table name to be created in db */

		/****************************************
		 * Optional parameters are declared below
		 ****************************************/
		OPTIONAL_PARAMS,

		DEBUG, /* if enabled trace file will be generated. */
		IMAGE_DOWNLOAD_PATH, /* image download path only requires if configured for downloading images. */

		RUN_SINGLE_PAGE, /* to run crawler for just one page */

		DB_OVERRIDE_TABLE, /* if set will drop table if already exists */

		CONF_IGNORE_LIST, /* ignore URL list */
		CONF_IGNORE_PATTERN_LIST, /* ignore url pattern list while processing */
		CONF_HTML_RULE, /* HTML rules */

		DELAY, /* Delay between processing each URL */

		/****************************************
		 * Parameters list end
		 ****************************************/
		PARAMETERS_END
	};

	/* mapping from enum to value */
	static string getParameterValue(int val) {
		switch (val) {
		case BASE_URL:
			return "baseURL";
		case DEBUG:
			return "debug";
		case IMAGE_DOWNLOAD_PATH:
			return "imagesDownloadPath";
		case RUN_SINGLE_PAGE:
			return "runSinglePage";
		case DB:
			return "DB";
		case DB_TYPE:
			return "type";
		case DB_USER:
			return "user";
		case DB_PASSWORD:
			return "password";
		case DB_NAME:
			return "dbName";
		case DB_PORT:
			return "port";
		case DB_TABLE_NAME:
			return "tableName";
		case DB_OVERRIDE_TABLE:
			return "overrideTable";
		case CONF_IGNORE_LIST:
			return "URLIgnoreList";
		case CONF_IGNORE_PATTERN_LIST:
			return "URLIgnorePatterList";
		case DELAY:
			return "delay";
		case CONF_HTML_RULE:
			return "htmlRule";
		}
		return "INVALID";
	}

	/* Check if parameters is optional/mandatory */
	static bool isOptional(int val) {
		if (val >= PARAMETERS_END || val < OPTIONAL_PARAMS)
			return false;
		return true;
	}

};

#endif
