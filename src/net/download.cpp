#define _GNU_SOURCE

#include "download.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/net/http.h>
#include <psp2/libssl.h>
#include <psp2/io/fcntl.h>
#include <curl/curl.h>

#include "../utils/filesystem.hpp"


CURL *curl;
int plFD;

void netInit() {
	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);

	SceNetInitParam netInitParam;
	int size = NET_INIT_SIZE;
	netInitParam.memory = malloc(size);
	netInitParam.size = size;
	netInitParam.flags = 0;
	sceNetInit(&netInitParam);

	sceNetCtlInit();
}

void netTerm() {
	sceNetCtlTerm();
	sceNetTerm();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
}

void httpInit() {
	sceSysmoduleLoadModule(SCE_SYSMODULE_SSL);
	sceSysmoduleLoadModule(SCE_SYSMODULE_HTTPS);
	sceHttpInit(NET_INIT_SIZE);
	sceSslInit(NET_INIT_SIZE);
}

void httpTerm() {
	sceSslTerm();
	sceHttpTerm();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTPS);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_SSL);
}

static size_t write_data_to_disk(void *ptr, size_t size, size_t nmemb, void *stream){
	size_t written = sceIoWrite( *(int*) stream, ptr, size*nmemb);
	return written;
}

void curlDownload(const char *url, const char *dest) {
	int plFD = sceIoOpen( dest , SCE_O_WRONLY | SCE_O_CREAT, 0777);

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl) {
		curl_easy_reset(curl);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_to_disk);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &plFD);

		curl_easy_perform(curl);
	}
	
	sceIoClose(plFD);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

std::string curlDownloadKeepName(char const*const url, std::string dst) {
	CURL *curl;

	SceUID file = sceIoOpen("ux0:data/SimpleVPK/plugin.tmp", SCE_O_CREAT | SCE_O_WRONLY, 0777);
	SceUID head = sceIoOpen("ux0:data/SimpleVPK/head.tmp", SCE_O_CREAT | SCE_O_WRONLY, 0777);

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36");
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 8L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data_to_disk);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &head);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_to_disk);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);

		curl_easy_perform(curl);
	}

	sceIoClose(file);
	sceIoClose(head);

	std::string header = Filesystem::readFile("ux0:data/SimpleVPK/head.tmp");

	if (header.find("filename=\"") != string::npos) {
		header = header.substr(header.find("filename=\"") + 10);
		header = header.substr(0, header.find("\""));
	} else if (header.find("location: ") != string::npos) {
		header = header.substr(header.find("location: ") + 10);
		header = header.substr(0, header.find("\n") - 1);

		while (header.find("/") != string::npos)
			header = header.substr(header.find("/") + 1);
	}

	Filesystem::copyFile("ux0:data/SimpleVPK/plugin.tmp", dst+header);

	sceIoRemove("ux0:data/SimpleVPK/plugin.tmp");
	sceIoRemove("ux0:data/SimpleVPK/head.tmp");

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return header;
}
