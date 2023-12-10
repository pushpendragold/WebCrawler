#include <curl/curl.h> 
#include <cassert>
#include <iostream>
#include <cstdio>
#include <vector>
#include <queue>
#include <fstream>
#include <map>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define USE_SITE_1 1

#include "WebSpider.h"
#include "WebSpider-conf.h"

using namespace std;

// Webspider
WebSpider * spider;
string WebSpider::curlData;

// Signal Handler
void signal_callback_handler(int signum);

// Usage Help
void Usage();

int main() {
	/*
	 TODO
	 1) Add configurablity without changing code.
	 2) Add timer to stop processing after timeout.
	 3) fail safe.
	 4) Save state when reached checkpoint.
	 5) Enable tracing on the run.

	 */
	//char URL[] = "http://www.bawarchi.com/";
	char URL[] = "http://allcompanieslist.com/companies-in-Business-Services";
	//char URL[]  = "http://www.imdb.com/";
	int depth = 10;

	/* Register signal and signal handler */
	signal(SIGINT, signal_callback_handler);

	/* MYSQL & POSTGRE DB are supported */
	spider = new WebSpider(URL, depth, MYSQL_DB);

#if TEST_SUIT

	// RUN TEST CASES
#if (USE_SITE_1 == 1)

	string singleURL("http://www.spoj.com/problems/VISIBLEBOX/");
	spider->Init_DBOptions("localhost", "ows", "123456", "ows", 3306);

	cout << "running for single page " << endl;
	cout << singleURL << endl;

	spider->EnableTrace(true, ALL);
	spider->SetImageDownloadLocation("/home/goldie/workspace/Crawler/");

	spider->SetTableName("test");
	spider->Init();

	/* DB Test */
	if (spider->Test_ConnectToServer()) {
		cout << "Connected to host";
	} else {
		cout << "Connection fail: Exiting" << endl;
		return 0;
	}

	// Rules
	spider->AddHTMLDataRule("Name",
			"<h2 id=\"problem-name\" class=\"text-center\">");
	spider->AddHTMLDataRule("Description", "<div id=\"problem-body\">");
	spider->RunSinglePage(singleURL);

#else
	/* allcompanieslist.com */
	string a =
	"<div class=\"banner_info\"><h2><a href=\"http://www.bawarchi.com/recipe/eggless-pumpkin-oats-and-nutella-muffins-oezqn6jbihjfb.html\">Nutella Muffins</a></h2><p><a href=\"http://www.bawarchi.com/recipe/eggless-pumpkin-oats-and-nutella-muffins-oezqn6jbihjfb.html\">This pumpkin bake is filled with oats and also these muffins are eggless. </a></p>";
	string singleURL("http://allcompanieslist.com/AROCHEM-SILVASSA-LIMITED");
	//string singleURL("http://allcompanieslist.com/companies-in-Business-Services");
	string A_test(a);
	string B(
			"http://ksmartstatic.sify.com/cmf-1.0.0/appflow/bawarchi.com/Image/oeru4lcfdaeeh_bigger.jpg");

	cout << spider->Test_StripHTML(A_test) << endl;
	cout << spider->Test_GetFileNameFromURL(B) << endl;

	spider->Init_DBOptions("localhost", "ows", "123456", "ows", 3306);
	// Testing Single Page
	cout << "running for single page " << endl;

	cout << singleURL << endl;

	spider->EnableTrace(true, ALL);
	spider->SetImageDownloadLocation("/home/goldie/workspace/Crawler/");
	//spider->Init_DBOptions("localhost","crawler","123456","ows",3306); // MYSQL
	spider->SetTableName("test");
	spider->setForceCreateTable(true);
	spider->setBaseWebsiteUrl("http://allcompanieslist.com");

	// Add URLs to ignore list
	spider->AddURLToVisitedList("http://allcompanieslist.com/index.aspx");
	spider->AddURLToVisitedList("http://allcompanieslist.com/About-us.aspx");
	spider->AddURLToVisitedList("http://allcompanieslist.com/contact-us.aspx");
	spider->AddURLToVisitedList("http://allcompanieslist.com/Cities.aspx");
	spider->AddURLToVisitedList("http://allcompanieslist.com/Industries.aspx");

	// Add Ignore Pattern List
	spider->AddIgnorePatternURLs("MNC-private-limited-companies-list-database");
	spider->AddIgnorePatternURLs("http://allcompanieslist.com/Maps/");
	spider->AddIgnorePatternURLs(
			"http://allcompanieslist.com/Similar-Companies/");
	spider->AddIgnorePatternURLs("http://allcompanieslist.com/Directors/");

	spider->setDelayBtwEachCall(30 * 1000);// Delay is in miliseconds
	spider->Init();

	/* DB Test */
	if (spider->Test_ConnectToServer()) {
		cout << "Connected to host";
	} else {
		cout << "Connection fail: Exiting" << endl;
		return 0;
	}

	// Rules
	HtmlDataTag baseHTML;
	baseHTML.setFullHtmLtag("<div class='box'>");
	baseHTML.setFieldSize(200);

	// Sub rules
	SubHTMLDataTag subHTML;
	vector<SubHTMLDataTag> subRules;

	// Adding subHTML rules (1)
	subHTML.setRuleHtmlTag("<div class='item-label'>Name </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("CompanyName");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	// Adding subHTML rules (2)
	subHTML.setRuleHtmlTag("<div class='item-label'>Industry </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("Industry");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Sub-Industry </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("SubIndustry");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>CIN </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("CIN");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>ROC Code </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("ROCcode");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Company Category </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("CompanyCategory");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag(
			"<div class='item-label'>Company Sub Category </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("CompanySubCategory");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Class Of Company </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("ClassOfCompany");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Authorised Capital </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("AuthorisedCapital");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Paid up Capital </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("PaidUpCapital");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Established In </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("EstablishedIn");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Address Line 1 </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("AddressLine1");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>State </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("State");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Country </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("Country");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Status </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("Status");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Listing Type </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("ListingType");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	// add Sub HTML data rule
	spider->AddSubHTMLDataRule(baseHTML, subRules);

	spider->RunSinglePage(singleURL);
#endif

#else // NOT TEST SUITE
#if (USE_SITE_1 == 1)
	spider->EnableTrace(true,ALL);
	spider->SetShowStatus(true);
	spider->SetImageDownloadLocation("/home/goldie/workspace/Crawler/test1/");

	spider->Init_DBOptions("localhost","crawler","123456","ows",3306); // MYSQL
	//spider->Init_DBOptions("127.0.0.1","postgres","","mydb",5432); // PostgreSQL
	spider->SetTableName("test5");
	spider->SetLimit(1000);// Set limit on number of DB entries
	spider->Init();

	// HTML Data
	spider->AddHTMLDataRule("Name","<span itemprop=\"name\">");
	spider->AddHTMLDataRule("Description","<p class=\"abt_prdct\" style=\"width:100%\" itemprop=\"description\" >",false);
	spider->AddHTMLDataRule("Ingredient","<div class=\"prdct_inf_hldr ingredients\">");
	spider->AddHTMLDataRule("Method","<ol class=\"number\" itemprop=\"recipeInstructions\" >",true,4000);
	spider->AddHTMLDataRule("Details","<div class=\"product_desc\">");

	// Replace HTML tag
	spider->ReplaceHTMLTagWith("<li>","::","Method");
	spider->ReplaceHTMLTagWith("<p itemprop=\"ingredients\">","::","Ingredient");
	spider->ReplaceHTMLTagWith("<dt>","::","Details");

	// IMG Download
	spider->AddSaveIMGRule("Image","class=\"img_lin\"");

	/*
	 cout << "running for single page " << endl;
	 string singleURL("http://www.bawarchi.com/recipe/green-apple-cinnamon-cake-oetaqOceghffb.html");
	 cout << singleURL << endl;
	 spider->RunSinglePage(singleURL);
	 */

	cout << "Loading queue\n";
	spider->LoadQueue("dump_queue");
	cout << "Loading status\n";
	spider->LoadStatus("dump_status");

	spider->Run();
#else

	spider->EnableTrace(true, ALL);
	spider->SetShowStatus(true);
	//spider->setForceCreateTable(true);

	spider->SetImageDownloadLocation("/home/goldie/workspace/Crawler/test1/");

	spider->Init_DBOptions("127.0.0.1", "crawler", "123456", "ows", 3306);// MYSQL
	spider->SetTableName("AllCompaniesList");
	spider->setBaseWebsiteUrl("http://allcompanieslist.com");

	spider->setDelayBtwEachCall(3 * 1000);// Delay is in miliseconds

	// Add URLs to ignore list
	spider->AddURLToVisitedList("http://allcompanieslist.com/index.aspx");
	spider->AddURLToVisitedList("http://allcompanieslist.com/About-us.aspx");
	spider->AddURLToVisitedList("http://allcompanieslist.com/contact-us.aspx");
	spider->AddURLToVisitedList("http://allcompanieslist.com/Cities.aspx");
	spider->AddURLToVisitedList("http://allcompanieslist.com/Industries.aspx");

	// Add Ignore Pattern List
	spider->AddIgnorePatternURLs("MNC-private-limited-companies-list-database");
	spider->AddIgnorePatternURLs("http://allcompanieslist.com/Maps/");
	spider->AddIgnorePatternURLs(
			"http://allcompanieslist.com/Similar-Companies/");
	spider->AddIgnorePatternURLs("http://allcompanieslist.com/Directors/");

	// Rules
	HtmlDataTag baseHTML;
	baseHTML.setFullHtmLtag("<div class='box'>");
	baseHTML.setFieldSize(200);

	// Sub rules
	SubHTMLDataTag subHTML;
	vector<SubHTMLDataTag> subRules;

	// Adding subHTML rules (1)
	subHTML.setRuleHtmlTag("<div class='item-label'>Name </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("CompanyName");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	// Adding subHTML rules (2)
	subHTML.setRuleHtmlTag("<div class='item-label'>Industry </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("Industry");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Sub-Industry </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("SubIndustry");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>CIN </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("CIN");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>ROC Code </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("ROCcode");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Company Category </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("CompanyCategory");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag(
			"<div class='item-label'>Company Sub Category </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("CompanySubCategory");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Class Of Company </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("ClassOfCompany");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Authorised Capital </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("AuthorisedCapital");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Paid up Capital </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("PaidUpCapital");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Established In </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("EstablishedIn");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Address Line 1 </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("AddressLine1");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>State </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("State");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Country </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("Country");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Status </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("Status");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	subHTML.setRuleHtmlTag("<div class='item-label'>Listing Type </div>");
	subHTML.setMustHave(true);
	subHTML.setRuleName("ListingType");
	subHTML.setDataHtmlTag("<div class='item-data'>");
	subRules.push_back(subHTML);

	// add Sub HTML data rule
	spider->AddSubHTMLDataRule(baseHTML, subRules);

	spider->Init();

	cout << "Loading queue\n";
	spider->LoadQueue("dump_queue");
	cout << "Loading status\n";
	spider->LoadStatus("dump_status");

	spider->Run();

#endif
#endif

	// Delete our spider
	delete spider;

	return EXIT_SUCCESS;
}

void Usage() {
	cout << "Use -h for help" << endl;
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void signal_callback_handler(int signum) {
	printf("Caught Terminating signal %d\n", signum);

	// Cleanup and close up stuff here
	spider->DoExitTask();

	exit(signum);
}
