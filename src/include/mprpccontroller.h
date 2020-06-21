#pragma once
#include <google/protobuf/service.h>
#include <string>

class MprpcController : public google::protobuf::RpcController
{
public:
    MprpcController();
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);

    // undo
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);

private:
    bool m_failed;  // state  ->  call rpc method
    std::string m_errText;  // errmsg -> call rpc method
};