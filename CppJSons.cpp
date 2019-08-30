// CppJSons.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <mutex>
#include <atomic>
#include <stdlib.h> 
#include <chrono>
#include <ratio>

#include "json.hpp"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"



using namespace rapidjson;

using namespace std;

#include "TJson.h"; 

#include "SrchNode.h"

class TestClls {
public:
	int iVl;
	void F1() {
		cout << "\nF1 body...";;
	}

	void F2() {
		cout << "\nF2 body...";
		F1();
		F3();
	}
	static void F3() {
		static int iF3 = 0;
		iF3++;
		cout << "\nF3 body... iF3: " << iF3;
	}
};

std::string GetJsonFromFile_(std::string strfile){
	std::string strJson = "";
	std::ifstream myfile(strfile);
	std::string line;
	if (myfile.is_open())
	{
		std::cout << "\n\nTest file, wait for reading in file:" << strfile;
		while (std::getline(myfile, line))
		{
			strJson += line;
		}
		myfile.close();
		std::cout << "\nstring json size: " << strlen(strJson.c_str());
	}
	else {
		std::cout << "Unable to open file: " << strfile;
	}
	return strJson;
}

double TestParseSpeedRapidjson(string &strJson){
	const clock_t begin_time = clock();
	Document document;
	document.Parse(strJson.c_str(), strJson.length());
	double dTime = float(clock() - begin_time) / CLOCKS_PER_SEC;
	//std::cout << "\nTestParseSpeedRapidjson: Time consume: " << dTime;
	return dTime;
}

double TestParseSpeedCjson(std::string &strJson){
	const clock_t begin_time = clock();
	auto js = nlohmann::json::parse(strJson.c_str());
	double dTime = float(clock() - begin_time) / CLOCKS_PER_SEC;
	//std::cout << "\nTestParseSpeedCjson: Time consume: " << dTime;
	return dTime;
}

void TestSpeed() {
	TestClls oTestClls;
	oTestClls.F2();
	oTestClls.F2();
	TestClls::F3();

	std::string canada = GetJsonFromFile("canada.json");
	std::string citm_catalog = GetJsonFromFile("citm_catalog.json");
	std::cout << "\n";
	double dT1 = 0, dT2 = 0, dT3 = 0, dT4 = 0;
	for (int i = 0; i < 100; i++) {
		dT1 += TestParseSpeedRapidjson(canada);
		dT2 += TestParseSpeedRapidjson(citm_catalog);

		dT3 += TestParseSpeedCjson(canada);
		dT4 += TestParseSpeedCjson(citm_catalog);
	}
	std::cout << "\nAverage times: T1 = " << dT1 / 100 << " T2: " << dT2 / 100 << " T3: " << dT3 / 100 << " T4: " << dT4 / 100;
}

void GetJsonText(Document &doc, string &strOut)
{
	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);
	doc.Accept(writer);
	auto output = buffer.GetString();
	strOut = output;
}

// http://rapidjson.org/
void ParserAndDump_RapidJson(std::string &canada, std::string &citm_catalog, atomic<int> &iTotal, atomic<int> &iFalseCount, mutex &mt, std::atomic<long long> &elapsed) {
	auto start = std::chrono::high_resolution_clock::now();
	Document canada_doc, citm_catalog_doc;

	string i_canada = canada, s_canada, i_citm_catalog = citm_catalog, s_citm_catalog;
	//canada_doc.Parse(i_canada.c_str(), i_canada.length());
	//citm_catalog_doc.Parse(i_citm_catalog.c_str(), i_citm_catalog.length());
	for (int i = 0; i < 10; i++) {
		canada_doc.Parse(i_canada.c_str(), i_canada.length());
		citm_catalog_doc.Parse(i_citm_catalog.c_str(), i_citm_catalog.length());
		
		GetJsonText(canada_doc, s_canada);
		GetJsonText(citm_catalog_doc, s_citm_catalog);
		
		if (s_canada != i_canada)
			iFalseCount++;
		if (s_citm_catalog != i_citm_catalog)
			iFalseCount++;
		iTotal ++;
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	elapsed += int_ms.count();
}


void TestThreadSafe_RapidJson() {
	std::string strCanadaSrc = GetJsonFromFile("canada.json");
	std::string strCitmCanadaSrc = GetJsonFromFile("citm_catalog.json");
	Document doc_canada, doc_citm_catalog;
	doc_canada.Parse(strCanadaSrc.c_str(), strCanadaSrc.length());
	doc_citm_catalog.Parse(strCitmCanadaSrc.c_str(), strCitmCanadaSrc.length());
	GetJsonText(doc_canada, strCanadaSrc);
	GetJsonText(doc_citm_catalog, strCitmCanadaSrc);

	mutex mt;
	atomic<int> iTotal; iTotal = 0;
	atomic<int> iFalseCount; iFalseCount = 0;
	ofstream myfile;
	myfile.open("radpijsonperftest.log");
	std::atomic<long long> elapsed;
	double dAvrElapsed;
	while (1) {
		elapsed = 0;

		//auto t1 = new thread(ParserAndDump_RapidJson, strCanadaSrc, strCitmCanadaSrc, std::ref(iTotal), std::ref(iFalseCount), std::ref(mt), std::ref(elapsed));
		//auto t2 = new thread(ParserAndDump_RapidJson, strCanadaSrc, strCitmCanadaSrc, std::ref(iTotal), std::ref(iFalseCount), std::ref(mt), std::ref(elapsed));
		//auto t3 = new thread(ParserAndDump_RapidJson, strCanadaSrc, strCitmCanadaSrc, std::ref(iTotal), std::ref(iFalseCount), std::ref(mt), std::ref(elapsed));
		auto t4 = new thread(ParserAndDump_RapidJson, strCanadaSrc, strCitmCanadaSrc, std::ref(iTotal), std::ref(iFalseCount), std::ref(mt), std::ref(elapsed));
		cout << "\nPlease wait for complete your threads.\n";
		//cout << "\nTestThreadSafe_RapidJson t1.detach()"; t1->detach();
		//cout << "\nTestThreadSafe_RapidJson t2.detach()"; t2->detach();
		//cout << "\nTestThreadSafe_RapidJson t3.detach()"; t3->detach();
		cout << "\nTestThreadSafe_RapidJson t4.join()\n"; t4->join();

		dAvrElapsed = double(elapsed / 4);
		cout << "\nTestThreadSafe_CJSon, iTotal: " << iTotal << " iFalseCount: " << iFalseCount << " dAvrElapsed: " << dAvrElapsed << " milliseconds";
		myfile << "\nTestThreadSafe_CJSon, iTotal: " << iTotal << " iFalseCount: " << iFalseCount << " dAvrElapsed: " << dAvrElapsed << " milliseconds";
	}
}

void ParserAndDump_CJson(std::string &canada, std::string &citm_catalog, atomic<int> &iTotal, atomic<int> &iFalseCount, mutex &mt, std::atomic<long long> &elapsed) {
	auto start = std::chrono::high_resolution_clock::now();
	string strCanadaSrc = canada, strCanadaTmp;
	string strCitmCanadaSrc = citm_catalog, strCitmCanadaTmp;
	nlohmann::json jsonCanada = nlohmann::json::parse(strCitmCanadaSrc.c_str());			// Must this procedure ??
	nlohmann::json jsonCitmCanada = nlohmann::json::parse(strCanadaSrc.c_str());			// Must this procedure ??
	for (int i = 0; i < 10; i++) {
		jsonCanada = nlohmann::json::parse(strCanadaSrc.c_str());			// Must this procedure ??
		jsonCitmCanada = nlohmann::json::parse(strCitmCanadaSrc.c_str());	// Must this procedure ??
		strCanadaTmp = jsonCanada.dump();
		strCitmCanadaTmp = jsonCitmCanada.dump();

		if (strCanadaTmp != strCanadaSrc)
			iFalseCount++;
		if (strCitmCanadaTmp != strCitmCanadaSrc)
			iFalseCount++;
		iTotal++;
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	elapsed += int_ms.count();
}

void TestThreadSafe_CJSon() {
	std::string strCanadaSrc = GetJsonFromFile("C:/canada.json");
	std::string strCitmCanadaSrc = GetJsonFromFile("C:/citm_catalog.json");
	auto jsonCanada = nlohmann::json::parse(strCanadaSrc.c_str());
	auto jsonCitmCanada = nlohmann::json::parse(strCitmCanadaSrc.c_str());
	strCanadaSrc = jsonCanada.dump();
	strCitmCanadaSrc = jsonCitmCanada.dump();

	mutex mt;
	atomic<int> iTotal; iTotal = 0;
	atomic<int> iFalseCount; iFalseCount = 0;
	ofstream myfile;
	myfile.open("njson_test.log");

	std::atomic<long long> elapsed;
	double dAvrElapsed;
	while (1) {
		//auto start = std::chrono::high_resolution_clock::now();
		elapsed = 0;
		//auto t1 = new thread(ParserAndDump_CJson, strCanadaSrc, strCitmCanadaSrc, std::ref(iTotal), std::ref(iFalseCount), std::ref(mt), std::ref(elapsed));
		//auto t2 = new thread(ParserAndDump_CJson, strCanadaSrc, strCitmCanadaSrc, std::ref(iTotal), std::ref(iFalseCount), std::ref(mt), std::ref(elapsed));
		//auto t3 = new thread(ParserAndDump_CJson, strCanadaSrc, strCitmCanadaSrc, std::ref(iTotal), std::ref(iFalseCount), std::ref(mt), std::ref(elapsed));
		auto t4 = new thread(ParserAndDump_CJson, strCanadaSrc, strCitmCanadaSrc, std::ref(iTotal), std::ref(iFalseCount), std::ref(mt), std::ref(elapsed));
		

		cout << "\nPlease wait for complete your threads.\n";
		//cout << "\nTestThreadSafe_CJSon t1.detach()"; t1->detach();
		//cout << "\nTestThreadSafe_CJSon t2.detach()"; t2->detach();
		//cout << "\nTestThreadSafe_CJSon t3.detach()"; t3->detach();
		cout << "\nTestThreadSafe_CJSon t4.join()\n"; t4->join();
		dAvrElapsed = double(elapsed / 4);
		cout << "\nTestThreadSafe_CJSon, iTotal: " << iTotal << " iFalseCount: " << iFalseCount << " dAvrElapsed: " << dAvrElapsed << " milliseconds";
		myfile << "\nTestThreadSafe_CJSon, iTotal: " << iTotal << " iFalseCount: " << iFalseCount << " dAvrElapsed: " << dAvrElapsed << " milliseconds";
		//TestThreadSafe_CJSon, iTotal: 40 iFalseCount: 0 dAvrElapsed: 5621 milliseconds
	}
	myfile.close();
}


void ParserAndDump_TJson(std::string &canada, std::string &citm_catalog, atomic<int> &iTotal, atomic<int> &iFalseCount, mutex &mt, std::atomic<long long> &elapsed) {
	auto start = std::chrono::high_resolution_clock::now();
	string strCanadaSrc = canada, strCanadaTmp;
	string strCitmCanadaSrc = citm_catalog, strCitmCanadaTmp;
	TJSON oTJSON, oTJSON2;
	int iSize = 0; auto cBuffer = new char[8000000];
	
	for (int i = 0; i < 10; i++) {
		oTJSON.RootParse(strCanadaSrc.c_str());
		oTJSON.Stringify(cBuffer, iSize);
		strCanadaTmp = cBuffer;

		oTJSON2.RootParse(strCitmCanadaSrc.c_str());
		oTJSON2.Stringify(cBuffer, iSize);
		strCitmCanadaTmp = cBuffer;

		if (strCanadaTmp != strCanadaSrc)
			iFalseCount++;
		if (strCitmCanadaTmp != strCitmCanadaSrc)
			iFalseCount++;
		iTotal++;
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	elapsed += int_ms.count();
	delete[]cBuffer;
}


void TestThreadSafe_TJSon() {
	TJSON oTJSON;// , oTJSON2;
	std::string strCanadaSrc = GetJsonFromFile("canada.json");// "C:/canada.json"); "..\\..\\..\\Tools\\_DataSamples\\canada.json" "Debug.json"
	
	TJsonParseRet oParseRs = oTJSON.RootParse(strCanadaSrc.c_str());
	int iSize = 0;  auto *cBuffers = new char[8000000];
	oTJSON.Stringify(cBuffers, iSize); 
	strCanadaSrc = cBuffers;

	std::string strCitmCanadaSrc = GetJsonFromFile("citm_catalog.json");// "C:/citm_catalog.json");// "..\\..\\..\\Tools\\_DataSamples\\citm_catalog.json" "Debug.json"
	oParseRs = oTJSON.RootParse(strCitmCanadaSrc.c_str());
	oTJSON.Stringify(cBuffers, iSize);
	strCitmCanadaSrc = cBuffers;
	delete[]cBuffers;

	mutex mt;
	atomic<int> iTotal; iTotal = 0;
	atomic<int> iFalseCount; iFalseCount = 0;
	ofstream myfile;
	myfile.open("tjson_test.log");

	std::atomic<long long> elapsed;
	double dAvrElapsed;
	while (1) {
		elapsed = 0;
		//auto t1 = new thread(ParserAndDump_TJson, strCanadaSrc, strCitmCanadaSrc, std::ref(iTotal), std::ref(iFalseCount), std::ref(mt), std::ref(elapsed));
		//auto t2 = new thread(ParserAndDump_TJson, strCanadaSrc, strCitmCanadaSrc, std::ref(iTotal), std::ref(iFalseCount), std::ref(mt), std::ref(elapsed));
		//auto t3 = new thread(ParserAndDump_TJson, strCanadaSrc, strCitmCanadaSrc, std::ref(iTotal), std::ref(iFalseCount), std::ref(mt), std::ref(elapsed));
		auto t4 = new thread(ParserAndDump_TJson, strCanadaSrc, strCitmCanadaSrc, std::ref(iTotal), std::ref(iFalseCount), std::ref(mt), std::ref(elapsed));


		cout << "\nPlease wait for complete your threads.\n";
		//cout << "\nTestThreadSafe_CJSon t1.detach()"; t1->detach();
		//cout << "\nTestThreadSafe_CJSon t2.detach()"; t2->detach();
		//cout << "\nTestThreadSafe_CJSon t3.detach()"; t3->detach();
		cout << "\nTestThreadSafe_CJSon t4.join()\n"; t4->join();
		dAvrElapsed = double(elapsed / 4);
		cout << "\nTestThreadSafe_CJSon, iTotal: " << iTotal << " iFalseCount: " << iFalseCount << " dAvrElapsed: " << dAvrElapsed << " milliseconds";
		myfile << "\nTestThreadSafe_CJSon, iTotal: " << iTotal << " iFalseCount: " << iFalseCount << " dAvrElapsed: " << dAvrElapsed << " milliseconds";
		//TestThreadSafe_CJSon, iTotal: 40 iFalseCount: 0 dAvrElapsed: 5621 milliseconds
	}
	myfile.close();
	
}
const int stackallocationsize = 1*1024;// 1048576;// 1024 * 1024;//in bytes
void TestMaxRecursiveCpp(int &iDeepth, int *pInput = nullptr) {
	iDeepth++;
	int StackMem[stackallocationsize];
	if (pInput == nullptr) {
		for (int i = 0; i < stackallocationsize; i++) {
			StackMem[i] = i*(i - 1);
		}
	}
	else {
		for (int i = 0; i < stackallocationsize; i++) {
			StackMem[i] = pInput[i] + i;
		}
	}
	
	if (iDeepth == 200) {
		cout << "\nSee mem and press enter to back roll"; getchar();
	}
	else {
		TestMaxRecursiveCpp(iDeepth, StackMem);
	}
}

struct snode {
	
	int istt;
	int *pvctids, c, s;
	int *pItems, ic, is;
	snode **cnodes;
};

//See more benchmark @ https://github.com/miloyip/nativejson-benchmark#parsing-time
int _tmain(int argc, _TCHAR* argv[])
{
	//Test_base_Json();
	//TestSpeed();
	//TestThreadSafe_RapidJson();
	//int iDeepth = 0;
	//TestMaxRecursiveCpp(iDeepth);
	// ==> Average times: T1 = 0.01544 T2: 0.00979 T3: 0.10347 T4: 0.02315

	//TestThreadSafe_CJSon(); //TestThreadSafe_CJSon, iTotal: 40 iFalseCount: 0 dAvrElapsed: 5621 milliseconds
	
	
	//TestThreadSafe_RapidJson();
	//TestThreadSafe_CJSon();
	
	//For Python # elapsed_time:  0.121300005913
	
	TestThreadSafe_TJSon(); // In debug build:	 TestThreadSafe_CJSon, iTotal: 37060 iFalseCount: 33354 dAvrElapsed: 432 milliseconds
							// In release build: TestThreadSafe_CJSon, iTotal: 3680 iFalseCount: 3312 dAvrElapsed: 110 milliseconds
	
	//std::string strSortJson = GetJsonFromFile("..\\..\\..\\Tools\\_DataSamples\\citm_catalog.json");
	//std::string strSortJson = GetJsonFromFile("..\\..\\..\\Tools\\_DataSamples\\canada.json");
	//for(int i = 0; i < 10000; i ++ ) 
	//	Test_TJS_Node_JS(strSortJson, true);
	
	//Test_TJS_Node("e:/SortJson.js");// "c:/citm_catalog.json" "c:/canada.json" "c:/bugs.json" "c:/SignalLocal.json" "c:/SignalRemote.json" "c:/SortJson.json"
	
	/*for (int i = 0; i < 100; i++) {
		Test_TJS_Node("c:/citm_catalog.json");
		Test_TJS_Node("c:/canada.json");
		//"C:/citm_catalog.json"
	}*/

	/*Test_Casting();
	cout << "\nsizeof(VectType): " << sizeof(VectType);
	cout << "\nsizeof(SNode): " << sizeof(SNode);
	std::cout << "\nPress enter to exist small app.";*/


	std::getchar();

	return 0;
}

