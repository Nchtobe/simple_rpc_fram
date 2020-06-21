add_compile_options(-std=c++11)
# userservice
#set(SRC_LIST userservice.cc ../user.pb.cc)
# friendservice
set(SRC_LIST friendservice.cc ../friend.pb.cc)

add_executable(provider ${SRC_LIST})

target_link_libraries(provider mprpc protobuf)