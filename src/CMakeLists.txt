add_compile_options(-std=c++11)

aux_source_directory(. SRC_LIST)

# add_library(mprpc SHARED ${SRC_LIST}) 之前生成了动态链接库，但是muduo网络库时静态链接库，两者连接会出现问题
add_library(mprpc ${SRC_LIST})

target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)