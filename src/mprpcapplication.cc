#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

MprpcConfig MprpcApplication::m_config;

void ShowArgsHelp() {
    std::cout << "format: command -i <configfile>" << std::endl;
}

void MprpcApplication::Init(int argc, char **argv){
    if(argc < 2) {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    std::string config_file;
    int c = 0;
    while((c = getopt(argc, argv, "i:")) != -1) {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // load configuration
    m_config.LoadConfigFile(config_file.c_str());

    /*
    std::cout << "rpcserver ip: " << m_config.Load("rpcserverip") << std::endl;
    std::cout << "rpcserver port: " << m_config.Load("rpcserverport") << std::endl;
    std::cout << "zookeeper ip: " << m_config.Load("zookeeperip") << std::endl;
    std::cout << "zookeeper port: " << m_config.Load("zookeeperport") << std::endl;
    */
}

MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::GetConfig() {
    return m_config;
}