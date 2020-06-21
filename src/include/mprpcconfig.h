#pragma once

#include <unordered_map>
#include <string>

// read configuration information
class MprpcConfig {
public:
    // parse configuration information
    void LoadConfigFile(const char *config_file);
    
    // query configuration information
    std::string Load(const std::string &key);

private:
    std::unordered_map<std::string, std::string> m_configMap;
    // remove the space before and after the string
    void Trim(std::string &src_buf);
};