#ifndef __PARSE_H__
#define __PARSE_H__

#include "net.h"
#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define LOADBSUC   "HTTP/1.0 200 Proxy list Loaded" CRLF CRLF
#define ADDBTIP    "HTTP/1.0 200 Proxy site Added" CRLF CRLF

#define H404       "HTTP/1.0 404 Not Found" CRLF CRLF
    
int checkproxy(const char *host);
void addpsite(const char *host);
int loadproxysite();

int parse(char* header);
size_t parse302(const char *location,char* buff);
size_t parse200(int length,char *buff);

#ifdef  __cplusplus
}
#endif

#endif