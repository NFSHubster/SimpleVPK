#include "../main.hpp"


#define NET_INIT_SIZE 1*1024*1024


void netInit();
void netTerm();
void httpInit();
void httpTerm();
void curlDownload(const char *url, const char *dest);
std::string curlDownloadKeepName(char const*const url, std::string dst);