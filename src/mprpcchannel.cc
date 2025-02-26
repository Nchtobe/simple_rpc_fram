#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include "mprpcapplication.h"
#include "mprpccontroller.h"
#include "zookeeperutil.h"

/*
header_size + service_name + method_name + args_size + args
*/
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    // args_size
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        //std::cout << "Serialize request error!" << std::endl;
        controller->SetFailed("Serialize request error!");
        return;
    }

    // rpc req header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        // std::cout << "Serialize rpc header error!" << std::endl;
        controller->SetFailed("Serialize rpc header error!");
        return;
    }

    // rpc req str
    std::string send_rpc_str;
    
    send_rpc_str.insert(0, std::string((char *)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    // print debug
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_src: " << rpc_header_str << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "-----------------------------------" << std::endl;

    // tcp program
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == 0)
    {
        // std::cout << "create socket error: " << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, "create socket error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    /* 
    // no include zookeeper
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    std::uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    */
    ZkClient zkCli;
    zkCli.Start();
    std::string method_path = "/" + service_name + "/" + method_name;
    std::string host_data = zkCli.GetData(method_name.c_str());
    if(host_data == "") {
        controller->SetFailed(method_name + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if(idx == -1) {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    server_addr.sin_port = htons(port);

    // connect rpc node
    if (connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        // std::cout << "connect error: " << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, "connect error! errno: %d", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    if (send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0) == -1)
    {
        // std::cout << "send error: " << errno << std::endl;

        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno: %d", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    // rpc response
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if ((recv_size = recv(clientfd, recv_buf, 1024, 0)) == -1)
    {
        // std::cout << "recv error: " << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno: %d", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    // std::string response_str(recv_buf, 0, recv_size);
    // if(!response->ParseFromString(response_str)) {
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        // std::cout << "parse error response_str: " << recv_buf << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, "parse error response_str! errno: %s", recv_buf);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }
    close(clientfd);
}