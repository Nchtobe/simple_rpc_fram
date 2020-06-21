#include <iostream>
#include <semaphore.h>

#include "zookeeperutil.h"
#include "mprpcapplication.h"

// global watcher
void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE) // connect success
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            sem_post(sem); // sem + 1
        }
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);
    }
}

// zkclient start connect zkserver
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    /*
    zookeeper_mt：多线程版本
    zookeeper的API客户端程序提供了三个线程
    1. API调用线程，只是做一些内存的开发等前提工作，就是完成zookeeper_init的工作，但是此时成功并不意味着zookeeper连接成功，连接时下面的线程执行的
    2. 网络I/O线程：这里面包含有pthread_create创建了一个线程，负责连接zookeeper，客户端并不需要高并发，所以是通过poll实现的
    3. watcher回调线程(包含有pthread_create创建了一个线程)，执行我们设置的global_watcher回调

    整个过程都是一个异步的过程，并不是同步阻塞等待的

    异步等待直到回调中type == ZOO_SESSION_EVENT且state == ZOO_CONNECTED_STATE时才给出响应
    */
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (m_zhandle == nullptr)
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem);
    std::cout << "zookeeper_init success!" << std::endl;
}

// create zknode
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (flag == ZNONODE)
    {
        flag = zoo_create(m_zhandle, path, data, datalen,
                          &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if (flag == ZOK)
        {
            std::cout << "znode create success... path: " << path << std::endl;
        }
        else
        {
            std::cout << "flag: " << flag << std::endl;
            std::cout << "znode create error... path: " << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    if(flag != ZOK) {
        std::cout << "get znode error... path: " << path << std::endl;
        return "";
    }
    else {
        return buffer;
    }
}