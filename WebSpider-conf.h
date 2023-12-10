#ifndef _H_WEB_SPIDER_CONF_H
#define _H_WEB_SPIDER_CONF_H

/* Default visited URL log file name */
#define DEF_VISITED_URL_LOG_FILE "visitedURL.txt"

/* Dump file */
#define QUEUE_DUMP_FILE "dump_queue"
#define STATUS_DUMP_FILE "dump_status"

#include <vector>
#include <string>

using namespace std;

/* Enable testing suit */
//#define TEST_SUIT 1
/* MAX buf Size */
#define BUF_SIZE 1010101

#include "HtmlDataTag.h"
#include "IMGDownloadTag.h"
#include "ReplaceHTMLTag.h"
#include "SubHTMLDataTag.h"
#include "CommonHTMLDataTag.h"


/* Handler */
typedef void (*p_handler)(char *);

#endif
