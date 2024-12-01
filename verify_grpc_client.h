#pragma once
#include <grpcpp/grpcpp.h>
#include <memory>
#include <atomic>
#include <queue>
#include "const.h"
#include "proto/message.grpc.pb.h"
#include "singleton.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class RPConPool {
 public:
  RPConPool(size_t poolSize, std::string host, std::string port);

  ~RPConPool();

  std::unique_ptr<VarifyService::Stub> GetConnection();

  void ReturnConnection(std::unique_ptr<VarifyService::Stub> context);

  void Close();

 private:
  std::atomic<bool> b_stop_;
  size_t poolSize_;
  std::string host_;
  std::string port_;
  // 阻塞队列
  std::queue<std::unique_ptr<VarifyService::Stub>> connections_;
  std::mutex mutex_;
  std::condition_variable cond_;
};


class VerifyGrpcClient : public Singleton<VerifyGrpcClient> {
  friend class Singleton<VerifyGrpcClient>;

 public:
  GetVarifyRsp GetVarifyCode(const std::string& email);

 private:
  VerifyGrpcClient();

  std::unique_ptr<RPConPool> pool_;
};
