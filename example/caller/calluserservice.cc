#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"

int main(int argc, char **argv)
{
    // init
    MprpcApplication::Init(argc, argv);

    fixbug::UserServiceRpc_Stub stub(new MprpcChannel);

    // rpc method args    login
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    // rpc response
    fixbug::LoginResponse response;

    // call rpc method
    stub.Login(nullptr, &request, &response, nullptr);

    if (response.result().errcode() == 0)
    {
        std::cout << "rpc login response success: " << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login response: " << response.result().errmsg() << std::endl;
    }

    // regist
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("wy");
    req.set_pwd("654321");
    fixbug::RegisterResponse rsp;
    
    stub.Register(nullptr, &req, &rsp, nullptr);

    if (rsp.result().errcode() == 0)
    {
        std::cout << "rpc regist response success: " << rsp.success() << std::endl;
    }
    else
    {
        std::cout << "rpc regist response: " << rsp.result().errmsg() << std::endl;
    }
    return 0;
}