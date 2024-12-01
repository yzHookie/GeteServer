#include "verify_grpc_client.h"
#include "config_mgr.h"
GetVarifyRsp VerifyGrpcClient::GetVarifyCode(const std::string& email) {
  ClientContext context;
  GetVarifyRsp reply;
  GetVarifyReq request;
  request.set_email(email);
  auto stub = pool_->GetConnection();
  Status status = stub->GetVarifyCode(&context, request, &reply);

  if (status.ok()) {
    pool_->ReturnConnection(std::move(stub));
    return reply;
  } else {
    pool_->ReturnConnection(std::move(stub));
    reply.set_error(ErrorCodes::RPCFailed);
    return reply;
  }
}

VerifyGrpcClient::VerifyGrpcClient() {
  auto& g_cfg_mgr = ConfigMgr::Inst();
  std::string host = g_cfg_mgr["VarifyServer"]["Host"];
  std::string port = g_cfg_mgr["VarifyServer"]["Port"];
  
  pool_.reset(new RPConPool(5, host, port));
}

RPConPool::RPConPool(size_t poolSize, std::string host, std::string port)
    : poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
  for (size_t i = 0; i < poolSize_; ++i) {
    std::shared_ptr<Channel> channel = grpc::CreateChannel(
        host + ":" + port, grpc::InsecureChannelCredentials());

    connections_.push(VarifyService::NewStub(channel));
  }
}

RPConPool::~RPConPool() {
  std::lock_guard<std::mutex> lock(mutex_);
  Close();
  while (!connections_.empty()) {
    connections_.pop();
  }
}

std::unique_ptr<VarifyService::Stub> RPConPool::GetConnection() {
  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [this] {
    if (b_stop_) {
      return true;
    }
    return !connections_.empty();
  });
  // 如果停止则直接返回空指针
  if (b_stop_) {
    return nullptr;
  }
  auto context = std::move(connections_.front());
  connections_.pop();
  return context;
}

void RPConPool::ReturnConnection(std::unique_ptr<VarifyService::Stub> context) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (b_stop_) {
    return;
  }
  connections_.push(std::move(context));
  cond_.notify_one();
}

void RPConPool::Close() {
  b_stop_ = true;
  cond_.notify_all();
}
