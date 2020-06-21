add_compile_options(-std=c++11)

# userservice
#set(SRC_LIST calluserservice.cc ../user.pb.cc)
# friendservice
set(SRC_LIST callfriendservice.cc ../friend.pb.cc)

add_executable(consumer ${SRC_LIST})

target_link_libraries(consumer mprpc protobuf)