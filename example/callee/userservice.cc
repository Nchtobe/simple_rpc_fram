#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

using namespace std;

class Userservice : public fixbug::UserServiceRpc
{
public:
    bool Login(string name, string pwd)
    {
        cout << "doing local service: Login" << endl;
        cout << "name: " << name << "pwd: " << pwd << endl;
        return true;
    }

    // local method
    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        cout << "doing local service: Login" << endl;
        cout << "id: " << id << "name: " << name << "pwd: " << pwd << endl;
        return true;
    }

    // rpc method
    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        std::string name = request->name();
        std::string pwd = request->pwd();

        // call local method
        bool login_result = Login(name, pwd);

        // response
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        // callback
        done->Run();
    }

    void Register(::google::protobuf::RpcController *controller,
                  const fixbug::RegisterRequest *request,
                  ::fixbug::RegisterResponse *response,
                  ::google::protobuf::Closure *done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool ret = Register(id, name, pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);

        done->Run();
    }
};

int main(int argc, char **argv)
{
    // init
    MprpcApplication::Init(argc, argv);

    // public service
    RpcProvider provider;
    provider.NotifyService(new Userservice());

    // start
    provider.Run();

    return 0;
}