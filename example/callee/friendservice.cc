#include <iostream>
#include <string>
#include <vector>

#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"

class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(uint32_t userid)
    {
        std::cout << "do GetFriendsList service, userid: " << userid << std::endl;

        std::vector<std::string> vec;
        vec.push_back("hhh");
        vec.push_back("jjj");
        vec.push_back("www");
        return vec;
    }

    void GetFriendsList(::google::protobuf::RpcController *controller,
                        const ::fixbug::GetFriendsListRequest *request,
                        ::fixbug::GetFriendsListResponse *response,
                        ::google::protobuf::Closure *done)
    {
        uint32_t userid = request->userid();
        std::vector<std::string> friendsList = GetFriendsList(userid);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");

        for(std::string name : friendsList) {
            std::string *p = response->add_friends();
            *p = name;
        }

        done->Run();
    }
};

int main(int argc, char **argv)
{
    LOG_INFO("first log message! ");
    LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    // init
    MprpcApplication::Init(argc, argv);

    // publish service
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // start
    provider.Run();

    return 0;
}