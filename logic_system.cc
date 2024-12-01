#include "logic_system.h"

#include "http_connection.h"
#include "verify_grpc_client.h"

LogicSystem::LogicSystem() {
  ReqGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
    beast::ostream(connection->response_.body()) << "receive get_test req";
    int index = 0;
    for (auto& elem : connection->get_params_) {
      index++;
      beast::ostream(connection->response_.body())
          << "param " << index << "key is " << elem.first;
      beast::ostream(connection->response_.body())
          << ", param " << index << "value is " << elem.second << std::endl;
    }
  });

  ReqPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
    auto body_str =
        boost::beast::buffers_to_string(connection->request_.body().data());
    spdlog::info("receive body is {}", body_str);
    connection->response_.set(http::field::content_type, "text/json");
    // 发送的root
    Json::Value root;
    Json::Reader reader;
    // 接收json 解析的root
    Json::Value src_root;
    bool parse_success = reader.parse(body_str, src_root);
    if (!parse_success) {
      spdlog::info("Failed to parse JSON data!");
      root["error"] = ErrorCodes::Error_Json;
      std::string jsonstr = root.toStyledString();
      beast::ostream(connection->response_.body()) << jsonstr;
      return true;
    }

    if (!src_root.isMember("email")) {
      spdlog::info("email is not in json");
      root["error"] = ErrorCodes::Error_Json;
      // tcp面向字节流，要转成字符串
      std::string jsonstr = root.toStyledString();
      beast::ostream(connection->response_.body()) << jsonstr;
      return true;
    }
    auto email = src_root["email"].asString();
    // 通过 grpc，向 varify 服务发请求
    GetVarifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVarifyCode(email);
    spdlog::info("email is {}", email);
    root["error"] = 0;
    root["email"] = src_root["email"];

    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return true;
  });
}
bool LogicSystem::HandleGet(const std::string& path,
                            std::shared_ptr<HttpConnection> con) {
  if (get_handlers_.find(path) == get_handlers_.end()) {
    return false;
  }
  get_handlers_[path](con);
  return true;
}
bool LogicSystem::HandlePost(const std::string& path,
                             std::shared_ptr<HttpConnection> con) {
  if (post_handlers_.find(path) == post_handlers_.end()) {
    return false;
  }
  post_handlers_[path](con);
  return true;
}
void LogicSystem::ReqGet(const std::string& url, HttpHandler handler) {
  get_handlers_.insert(std::make_pair(url, handler));
}

void LogicSystem::ReqPost(const std::string& url, HttpHandler handler) {
  post_handlers_.insert(std::make_pair(url, handler));
}
