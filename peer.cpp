#include "peer.h"
#include "guest.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>


Peer::Peer(int fd):Con(fd) {
}


Peer::~Peer() {
    while(!write_queue.empty()){
        free(write_queue.front().buff);
        write_queue.pop();
    }
}

ssize_t Peer::Write(const void* buff, size_t size, Peer* who, uint32_t) {
    if(size == 0) {
        return 0;
    }
    void *dup_buff = malloc(size);
    memcpy(dup_buff, buff, size);
    return Peer::Write(dup_buff, size, who);
}

ssize_t Peer::Write(void* buff, size_t size, Peer* , uint32_t) {
    if(size == 0) {
        return 0;
    }
    
    write_block wb={buff, size, 0};
    write_queue.push(wb);
    writelen += size;

    updateEpoll(EPOLLIN | EPOLLOUT);
    return size;
}

ssize_t Peer::Read(void* buff, size_t size) {
    return read(fd, buff, size);
}

ssize_t Peer::Write(const void* buff, size_t size) {
    return write(fd, buff, size);
}

int Peer::Write() {
    bool writed = false;
    while(!write_queue.empty()){
        write_block *wb = &write_queue.front();
        ssize_t ret = Write((char *)wb->buff + wb->wlen, wb->len - wb->wlen);

        if (ret <= 0) {
            return ret;
        }

        writed = true;
        writelen -= ret;
        if ((size_t)ret + wb->wlen == wb->len) {
            free(wb->buff);
            write_queue.pop();
        } else {
            wb->wlen += ret;
            return WRITE_INCOMP;
        }
    }

    updateEpoll(EPOLLIN);
    return writed ? WRITE_COMPLETE : WRITE_NOTHING;
}

void Peer::writedcb(Peer *) {
    updateEpoll(EPOLLIN | EPOLLOUT);
}

int32_t Peer::bufleft(Peer *) {
    if(writelen >= 1024*1024)
        return 0;
    else
        return BUF_LEN;
}


void Peer::clean(uint32_t errcode, Peer* , uint32_t) {
    reset_this_ptr();
    if(fd > 0) {
        updateEpoll(EPOLLOUT);
        handleEvent = (void (Con::*)(uint32_t))&Peer::closeHE;
    }
}

void Peer::wait(Peer *who) {
    epoll_ctl(efd, EPOLL_CTL_DEL, who->fd, NULL);
}
