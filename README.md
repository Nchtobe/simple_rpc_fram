# simple_rpc_fram
rpc+protobuf+zookeeper

### 文件夹介绍
#### bin
1. 生成的可执行文件，服务端和客户端
2. 配置文件，配置rpc节点的ip地址和端口，配置zookeeper的ip地址和端口

#### build
cmake编译产生的额外的文件

#### example
1. proto文件，对于不同的服务定义不同的message，并且根据message定义不同的服务service。service直接定义为rpc类型，接收的是request类型，返回response类型数据
2. 在命令行执行：proc xxx.proto --cpp_out=./   // 生成对应的C++接口文件
3. callee，rpc服务发布，完成的工作是（friendservice和userservice完成一样的工作，只不过是不同的服务）：
  1. rpc框架和zookeeper的初始化操作，就是读取配置文件的ip和端口号，加载给他们
  2. 定义一个rpc服务发布对象，同时调用该对象的NotifyService
