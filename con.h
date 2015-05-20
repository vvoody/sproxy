#ifndef CON_H__
#define CON_H__

extern int efd;

class Con {
public:
    void (Con::*handleEvent)(uint32_t events)=nullptr;
    virtual ~Con() {}
};


#endif
