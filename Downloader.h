#include <iostream>
#include <windows.h>
#include <wininet.h>
#include <cstdio>

const size_t MAX_FILENAME_SIZE = 512;
const size_t BUFF_SIZE = 1024;

class DldException {
    char _error[80];

  public:
	DldException(char* exception);
    const char* getError(); 
};

class Downloader {
	static bool isHttp (char* url);
	static bool httpVersionOk(HINTERNET hIurl);
	static bool getFilename(char* url, char* fname);
	static unsigned long openFile(char* url, bool reload, std::ofstream& fout);

  public:
    static bool download(char* url, bool restart=false, void(*update)(unsigned long, unsigned long)=NULL); 
}; 