#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Buffer.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

#include "google/protobuf/service.h"
#include "logger.h"

// rpc service
class RpcProvider
{
public:
    // publish rpc method function interface
    void NotifyService(google::protobuf::Service *service);

    // start rpc service node
    void Run();

private:
    // EventLoop
    muduo::net::EventLoop m_eventLoop;

    // service info
    struct ServiceInfo {
        google::protobuf::Service *m_service;   // save service object
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_mthodMap;  // save service method
    };

    // store <service, service::method>
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;
    // onConnection callback func
    void onConnection(const muduo::net::TcpConnectionPtr &);
    // onMessage callback func
    void onMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);
    // Closure callback 
    void sendRpcResponse(const muduo::net::TcpConnectionPtr &, google::protobuf::Message*);
};