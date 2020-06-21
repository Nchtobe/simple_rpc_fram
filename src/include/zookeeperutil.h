#pragma once

#include <semaphore.h>  //信号量
#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient {
public:
    ZkClient();
    ~ZkClient();
    // zkclient start to connect zkserver
    void Start();
    // create zknode
    void Create(const char *path, const char *data, int datalen, int state=0);
    std::string GetData(const char *path);
private:
    // zkhandle
    zhandle_t *m_zhandle;
};