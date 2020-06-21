#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "zookeeperutil.h"

#include <zookeeper/zookeeper.h>

// publish rpc method function interface
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // get service_name
    std::string service_name = pserviceDesc->name();
    // get method_count
    int methodCnt = pserviceDesc->method_count();

    // std::cout << "service name: " << service_name << std::endl;
    LOG_INFO("service name: %s", service_name.c_str());

    for (int i = 0; i < methodCnt; i++)
    {
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_mthodMap.insert({method_name, pmethodDesc});
        //std::cout << "method_name: " << method_name << std::endl;
        LOG_INFO("method name: %s", method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

// start rpc node
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    std::uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // create tcp object
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    // set callback 
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // thread number
    server.setThreadNum(4);

    // regist rpc node
    ZkClient zkCli;
    zkCli.Start();

    for(auto &sp : m_serviceMap) {
        //  service_name
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for(auto &mp : sp.second.m_mthodMap) {
            // /service_name/method_name
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }

    }

    std::cout << "RpcServer start, ip: " << ip << ", port: " << port << std::endl;

    // start network service 
    server.start();
    m_eventLoop.loop();
}

void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        conn->shutdown();
    }
}

void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp)
{
    // rpc res
    std::string recv_buf = buffer->retrieveAllAsString();

    // parse
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);

    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        std::cout << "rpc_header_src: " << rpc_header_str << " parse error!" << std::endl;
        return;
    }

    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // print
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_src: " << rpc_header_str << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "-----------------------------------" << std::endl;

    // get service object and method object
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_mthodMap.find(method_name);
    if (mit == it->second.m_mthodMap.end())
    {
        std::cout << service_name << " : " << method_name << " is not exist!" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service;
    const google::protobuf::MethodDescriptor *method = mit->second;

    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        std::cout << "request parse error content: " << args_str << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                    const muduo::net::TcpConnectionPtr&,
                                                                    google::protobuf::Message*>
                                                                    (this, 
                                                                    &RpcProvider::sendRpcResponse, 
                                                                    conn, response);

    service->CallMethod(method, nullptr, request, response, done);
}

// Closure callback
void RpcProvider::sendRpcResponse(const muduo::net::TcpConnectionPtr &conn,
                                  google::protobuf::Message *response)
{
    std::string response_str;
    if(response->SerializeToString(&response_str)) {
        conn->send(response_str);
    }
    else {
        std::cout << "serialize response_str error!" << std::endl;
    }
    conn->shutdown();
}