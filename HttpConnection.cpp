#include "HttpConnection.h"

#include "LogicSystem.h"
// socket 只有移动构造
HttpConnection::HttpConnection(tcp::socket socket)
    : socket_(std::move(socket)) {}

void HttpConnection::Start() {
  auto self = shared_from_this();
  http::async_read(socket_, buffer_, request_,
                   [self](beast::error_code ec, std::size_t bytes_transferred) {
                     try {
                       if (ec) {
                         spdlog::info("http read err is {}", ec.what());
                         return;
                       }
                       boost::ignore_unused(bytes_transferred);
                       self->HandleReq();
                       // 启动超时检测
                       self->CheckDeadline();
                     } catch (std::exception& exp) {
                       spdlog::info("exception is {}", exp.what());
                     }
                   });
}

void HttpConnection::CheckDeadline() {
  auto self = shared_from_this();
  deadline_.async_wait([self](beast::error_code ec) {
    if (!ec) {
      self->socket_.close();
    }
  });
}

void HttpConnection::WriteResponse() {
  auto self = shared_from_this();
  response_.content_length(response_.body().size());
  http::async_write(
      socket_, response_,
      [self](beast::error_code ec, std::size_t bytes_transferred) {
        // 服务器关掉，服务器->客户端这一个方向的连接
        self->socket_.shutdown(tcp::socket::shutdown_send, ec);
        self->deadline_.cancel();
      });
}

void HttpConnection::HandleReq() {
  // 设置版本
  response_.version(request_.version());
  response_.keep_alive(false);
  if (request_.method() == http::verb::get) {
    bool success = LogicSystem::GetInstance()->HandleGet(request_.target(),
                                                         shared_from_this());
    if (!success) {
      response_.result(http::status::not_found);
      response_.set(http::field::content_type, "text/plain");
      beast::ostream(response_.body()) << "url not found\r\n";
      WriteResponse();
      return;
    }
    response_.result(http::status::ok);
    response_.set(http::field::server, "GetServer");
    WriteResponse();
    return;
  }
}