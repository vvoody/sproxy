#ifndef GUEST_H__
#define GUEST_H__

#include <netinet/in.h>

#include "peer.h"
#include "http.h"


class Guest:public Peer, public Http{
protected:
    char sourceip[INET6_ADDRSTRLEN];
    uint16_t  sourceport;
    
    char destip[INET6_ADDRSTRLEN];
    uint16_t  destport;

    virtual int showerrinfo(int ret, const char *)override;
    virtual void defaultHE(uint32_t events);
    virtual void closeHE(uint32_t events)override;
    
    virtual ssize_t Read(void* buff, size_t len)override;
    virtual void ErrProc(int errcode)override;
    virtual void ReqProc(HttpReqHeader &req)override;
    virtual ssize_t DataProc(const void *buff, size_t size)override;
public:
    Guest();
    explicit Guest(int fd);
    virtual void disconnected(Peer *who)override{};
    virtual void Response(HttpResHeader& res);
};

#endif
