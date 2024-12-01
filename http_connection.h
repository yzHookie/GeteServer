#pragma once
#include "const.h"

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
 public:
  friend class LogicSystem;
  HttpConnection(boost::asio::io_context& ioc);
  void Start();
  tcp::socket& GetSocket();
 private:
  // 检测是否超时
  void CheckDeadline();
  // 三次握手四次挥手的应答操作
  void WriteResponse();
  // 处理请求
  void HandleReq();
  // 解析请求参数
  void PreParseGetParam();

  tcp::socket socket_;
  beast::flat_buffer buffer_{8192};
  http::request<http::dynamic_body> request_;
  http::response<http::dynamic_body> response_;

  // 参数解析
  std::string get_url_;
  std::unordered_map<std::string, std::string> get_params_;

  // 定时器
  net::steady_timer deadline_{socket_.get_executor(), std::chrono::seconds(60)};
};
