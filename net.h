#ifndef NET__H__
#define NET__H__

#include <netdb.h>
#include <arpa/inet.h>

#define HTTPSPORT 443
#define HTTPPORT  80
#define CRLF      "\r\n"


#define connecttip  "HTTP/1.0 200 Connection established" CRLF CRLF

#define H200        "HTTP/1.0 200 OK" CRLF CRLF

#define H206        "HTTP/1.0 206 Partial Content" CRLF CRLF

#define H400        "HTTP/1.1 400 Bad Request" CRLF \
                    "Content-Length:0" CRLF CRLF

#define H403        "HTTP/1.1 403 Forbidden" CRLF \
                    "Content-Length: 0" CRLF CRLF

#define H404        "HTTP/1.1 404 Not Found" CRLF\
                    "Content-Length: 0" CRLF CRLF

#define H405        "HTTP/1.1 405 Method Not Allowed" CRLF\
                    "Content-Length:0" CRLF CRLF

#define H408        "HTTP/1.1 408 Request Timeout" CRLF\
                    "Content-Length:0" CRLF CRLF

#define H416        "HTTP/1.1 416 Requested Range Not Satisfiable" CRLF\
                    "Content-Length: 0" CRLF CRLF

#define H500        "HTTP/1.1 500 Internal Server Error" CRLF\
                    "Content-Length: 0" CRLF CRLF

#define CHUNCKEND "0" CRLF CRLF
#ifdef  __cplusplus
extern "C" {
#endif
    
extern const char *DEFAULT_CIPHER_LIST;

union sockaddr_un{
    struct sockaddr addr;
    struct sockaddr_in addr_in;
    struct sockaddr_in6 addr_in6;
};


int Bind_any(int fd, short port);
int Listen(short int port);
int Connect(union sockaddr_un*, int type);
const char *getaddrstring(union sockaddr_un *addr);
const char* getlocalip ();


#ifdef  __cplusplus
}
#endif

#endif
