#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>
#include <winInet.h>
#pragma comment ( lib, "Wininet.lib" )

#ifndef __DOWNLOADER__
#define __DOWNLOADER__
	#include "Downloader.h"
#endif

void statusBar(unsigned long total, unsigned long part) {
	size_t value = (size_t)((double)part/total*100);
	std::cout << value << "% complete." << std::endl;
}


size_t main(size_t argc, char** argv) {
   
	std::cout << "\t\t*** Web File Downloader ***\n\n"; 
	
	bool reload = true;
	std::string url("http://www.dracogne.ru");

	std::cout << "Enter URL or 'q' to qiut: ";
	std::cin >> url; 

	if(url == "q" || url == "Q") { std::cout << "Exit...\n\n"; return -1; }

	if(argc == 2 && !strcmp(argv[1], "no-reload")) reload = false;

	std::cout << "\nStart downloading...\n";

	try {
		
		if(Downloader::download((char*)url.c_str(), reload, statusBar)) std::cout << "\ndownload Complete" << std::endl;

	} catch(DldException e) {
		std::cout << e.getError() << "\nDownload interrupted!" << std::endl;	
	}








	system("pause"); return 0;
}