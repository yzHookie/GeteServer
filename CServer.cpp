#include "CServer.h"

#include "HttpConnection.h"
CServer::CServer(boost::asio::io_context& ioc, unsigned short& port)
    : ioc_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), port)), socket_(ioc) {}

void CServer::Start() {
  auto self = shared_from_this();
  // 防止回调时 this 被回调
  acceptor_.async_accept(socket_, [self](beast::error_code ec) {
    try {
      // 出错就放弃这个连接，继续监听其他连接
      if (ec) {
        self->Start();
        return;
      }
      // 在专门监听读写的连接中, HttpConnection 管理这个连接
      std::make_shared<HttpConnection>(std::move(self->socket_))->Start();
      // 继续监听，否则只能有一个客户端连接
      self->Start();
    } catch (std::exception& exp) {
    }
  });
}