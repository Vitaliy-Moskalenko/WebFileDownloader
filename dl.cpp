#include <cstdio>
#include <fstream>
#include <iostream>
#include <windows.h>
#include <winInet.h>
#pragma comment ( lib, "Wininet.lib" )

#ifndef __DOWNLOADER__
#define __DOWNLOADER__
	#include "Downloader.h"
#endif

/* ============== winInet.h Functions Used  =================================
InternetAttemptConnect  : Checks if the internet connection available
InternetOpen            : Establish i-net connection and returns its handler
InternetOpenUrl         : Opens URL and returns its handler
HttpQueryInfo           : Gets http-header data from the last response
InternetReadFile        : Reads bytes from opened URL
InternetCloseHandle     : Close i-net connection's handler
*/

DldException::DldException(char* exc) { if(strlen(exc) < 80) strcpy(_error, exc); }
const char* DldException::getError() { return _error; }

//Loads file, passes file URL to 'url'-parameter, passes 'true' to reload parameter
bool Downloader::download(char* url, bool restart, void(*update)(unsigned long, unsigned long)) {
	std::ofstream fout;      
	unsigned char buffer[BUFF_SIZE];
	unsigned long numrcved;            //Qty of bytes recieved
	unsigned long filelen;             //File length on HDD
	HINTERNET hIurl, hInet;            //Internet handlers
	unsigned long contentlen;         
	unsigned long len;
	unsigned long total = 0;           //Accumulated bytes qty
	char header[80];                   //contains Range header 

	try {
		if(!Downloader::isHttp(url)) throw DldException("Must be HTTP url.");

		filelen = Downloader::openFile(url, restart, fout);
		//Is connection available?
		if(InternetAttemptConnect(0) != ERROR_SUCCESS) throw DldException("Can't connect."); 
		//Open internet connection
		hInet = InternetOpen("downloader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
		if(!hInet) throw DldException("Can't open connection.");
		//Construct header of request data interval
		sprintf(header, "Range:bytes=%d-", filelen);
		//Opens URL and requests range
		hIurl = InternetOpenUrl(hInet, url, header, -1, INTERNET_FLAG_NO_CACHE_WRITE, 0);

		if(hIurl == NULL) throw DldException("Can't open Url.");

		if(!httpVersionOk(hIurl)) throw DldException("HTTP/1.1 not supported.");
		//Gets content length
		len = sizeof(contentlen);
		if(!HttpQueryInfo(hIurl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
			             &contentlen, &len, NULL)) throw DldException("File or content length not found.");

		//If existed file not complete - finishes its download
		if(filelen != contentlen && contentlen)
			do {
				if(!InternetReadFile(hIurl, &buffer, BUFF_SIZE, &numrcved))
					throw DldException("Error occurred during download.");  //Reads Info buffer

				fout.write((const char*)buffer, numrcved); //Writes buffer to HDD
				if(!fout.good()) throw DldException("Error writing file.");

				total += numrcved;
			
			} while (numrcved > 0);
		else { 
			if(update) update(filelen, filelen);
		}
	
	} catch(DldException) {
		fout.close();
		InternetCloseHandle(hIurl);
		InternetCloseHandle(hInet);

		throw; //Generate exception for caller
	} 

		fout.close();
		InternetCloseHandle(hIurl);
		InternetCloseHandle(hInet);

		return true;
}
//return true if http version 1.1 or later
bool Downloader::httpVersionOk(HINTERNET hIurl) { 
	char str[80];
	unsigned long len = 79;
	//Gets http version
	if(!HttpQueryInfo(hIurl, HTTP_QUERY_VERSION, &str, &len, NULL)) return false;

	char* p = strchr(str, '/'); p++;
	if(*p == '0') return false; // http 0.x
	p = strchr(str, '.'); p++;
	size_t minorVerNum = atoi(p);

	return minorVerNum > 0 ? true : false;
}
//Extracts file name from URL, return false if cant extract
bool Downloader::getFilename(char* url, char* fname) {
	char* p = strrchr(url, '/'); //Find last '/' char
	if(p && (strlen(p) < MAX_FILENAME_SIZE)) {
		p++; 
		strcpy(fname, p);
	    return true;
	}
	else return false;
}
//Open out-stream, returns file lenght? if reload=true erases content from existing file
unsigned long Downloader::openFile(char* url, bool reload, std::ofstream& fout) {
	char fname[MAX_FILENAME_SIZE];
	if(!getFilename(url, fname)) throw DldException("File name error.");

	if(!reload) fout.open(fname, std::ios::binary | std::ios::out | std::ios::app | std::ios::ate);
	else fout.open(fname, std::ios::binary | std::ios::out | std::ios::trunc);

	if(!fout) throw DldException("Can't open output file");
	//Return current file length
	return fout.tellp();
}

bool Downloader::isHttp(char* url) {
	char str[5] = "";
	strncpy(str, url, 4);

	for(char* p=str; *p; ++p) *p = tolower(*p);

	return !strcmp("http", str);
}
