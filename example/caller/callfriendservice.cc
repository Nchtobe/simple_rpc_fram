#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"

int main(int argc, char **argv)
{
    MprpcApplication::Init(argc, argv);

    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel);

    // rpc method args
    fixbug::GetFriendsListRequest request;
    request.set_userid(1000);

    // rpc response
    fixbug::GetFriendsListResponse response;

    MprpcController controller;

    // rpc request
    stub.GetFriendsList(&controller, &request, &response, nullptr);

    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (response.result().errcode() == 0)
        {
            std::cout << "rpc GetFriendsList response success: " << std::endl;
            int size = response.friends_size();
            for (int i = 0; i < size; i++)
            {
                std::cout << "index: " << (i + 1) << " name: " << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc GetFriendsList response: " << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}