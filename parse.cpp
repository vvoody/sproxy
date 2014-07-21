#include <string.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_set>

#include <arpa/inet.h>

#include "parse.h"

#define H302FORMAT "HTTP/1.1 302 Found" CRLF "Location: %s" CRLF "Content-Length: 0" CRLF CRLF
#define H200FORMAT "HTTP/1.1 200 OK" CRLF "Content-Length: %d" CRLF CRLF

#define PROXYFILE "proxy.list"

using namespace std;

static int loadedsite = 0;
static unordered_set<string> proxylist;


// trim from start
static inline string& ltrim(std::string && s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}


int loadproxysite() {
    loadedsite = 1;
    proxylist.clear();
    ifstream proxyfile(PROXYFILE);

    if (proxyfile.good()) {
        while (!proxyfile.eof()) {
            string site;
            proxyfile >> site;

            if(!site.empty()) {
                proxylist.insert(site);
            }
        }

        proxyfile.close();
        return proxylist.size();
    } else {
        cerr << "There is no " << PROXYFILE << "!" << endl;
        return -1;
    }
}


void addpsite(const string& host) {
    proxylist.insert(host);
    ofstream proxyfile(PROXYFILE);

    for(auto i : proxylist) {
        proxyfile << i << endl;
    }
}


bool Http::checkproxy() {
    if (!loadedsite) {
        loadproxysite();
    }

    //如果proxylist里面有*.*.*.* 那么ip地址直接代理
    if(inet_addr(hostname) != INADDR_NONE &&
       proxylist.find("*.*.*.*") != proxylist.end()) {
        return true;
    }

    const char* subhost = hostname;

    while (subhost) {
        if(subhost[0] == '.') {
            subhost++;
        }

        if (proxylist.find(subhost) != proxylist.end()) {
            return true;
        }

        subhost = strpbrk(subhost, ".");
    }

    return false;
}

char* toUpper(char* s) {
    char* p=s;

    while(*p) {
        *p=toupper(*p);
        p++;
    }

    return s;
}


int spliturl(const char* url, char* hostname, char* path , int* port) {
    const char* addrsplit;
    char tmpaddr[DOMAINLIMIT];
    int urllen = strlen(url);
    int copylen;
    bzero(hostname, DOMAINLIMIT);
    bzero(path, urllen);

    if (strncasecmp(url, "https://", 8) == 0) {
        url += 8;
        urllen -= 8;
        *port = HTTPSPORT;
    } else if (strncasecmp(url, "http://", 7) == 0) {
        url += 7;
        urllen -= 7;
        *port = HTTPPORT;
    } else if (!strstr(url, "://")) {
        *port = HTTPPORT;
    } else {
        return -1;
    }

    if ((addrsplit = strpbrk(url, "/"))) {
        copylen = url + urllen - addrsplit < (URLLIMIT - 1) ? url + urllen - addrsplit : (URLLIMIT - 1);
        memcpy(path, addrsplit, copylen);
        copylen = addrsplit - url < (DOMAINLIMIT - 1) ? addrsplit - url : (DOMAINLIMIT - 1);
        strncpy(tmpaddr, url, copylen);
        tmpaddr[copylen] = 0;
    } else {
        copylen = urllen < (DOMAINLIMIT - 1) ? urllen : (DOMAINLIMIT - 1);
        strncpy(tmpaddr, url, copylen);
        strcpy(path, "/");
        tmpaddr[copylen] = 0;
    }

    if (tmpaddr[0] == '[') {                                //this is a ipv6 address
        if (!(addrsplit = strpbrk(tmpaddr, "]"))) {
            return -1;
        }

        strncpy(hostname, tmpaddr + 1, addrsplit - tmpaddr - 1);

        if (addrsplit[1] == ':') {
            if(sscanf(addrsplit + 2, "%d", port) != 1)
                return -1;
        } else if (addrsplit[1] != 0) {
            return -1;
        }
    } else {
        if ((addrsplit = strpbrk(tmpaddr, ":"))) {
            strncpy(hostname, url, addrsplit - tmpaddr);

            if(sscanf(addrsplit + 1, "%d", port) != 1)
                return -1;
        } else {
            strcpy(hostname, tmpaddr);
        }
    }

    return 0;
}


Http::Http(char* header)throw (int){
    *(strstr(header, CRLF CRLF) + strlen(CRLF)) = 0;
    memset(path,0,sizeof(path));
    memset(url,0,sizeof(url));
    sscanf(header, "%s%*[ ]%[^\r\n ]", method, url);
    toUpper(method);

    if(spliturl(url,hostname,path,&port)){
        fprintf(stderr,"wrong url format:%s\n",url);
        throw 0;
    }

    for (char* str = strstr(header, CRLF) + strlen(CRLF); ; str = NULL) {
        char* p = strtok(str, CRLF);

        if (p == NULL)
            break;

        char* sp = strpbrk(p, ":");
        this->header[string(p, sp - p)] = ltrim(string(sp + 1));
    }

}

int Http::getstring( char* buff,bool shouldproxy) {
    int p;
    if(shouldproxy) {
        sprintf(buff, "%s %s HTTP/1.1" CRLF "%n",
                method,url, &p);
    } else {
        sprintf(buff, "%s %s HTTP/1.1" CRLF "%n",
                method, path, &p);
    }

    for (auto i : header) {
        int len;
        sprintf(buff + p, "%s:%s" CRLF "%n", i.first.c_str(), i.second.c_str(), &len);
        p += len;
    }

    sprintf(buff + p, CRLF);
    return p + strlen(CRLF);
}

string Http::getval(const char* key) {
    return header[key];
}


bool Http::ismethod(const char* method) {
    return strcmp(this->method,method)==0;
}


size_t parse302(const char* location, char* buff) {
    sprintf(buff, H302FORMAT, location);
    return strlen(buff);
}

size_t parse200(int length, char* buff) {
    sprintf(buff, H200FORMAT, length);
    return strlen(buff);
}



