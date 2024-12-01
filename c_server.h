#pragma once
#include "const.h"

// 直接使用 std::shared_ptr(this)导致多个独立的 shared_ptr 管理同一个对象
class CServer : public std::enable_shared_from_this<CServer> {
 public:
  // ioc 是与底层读写通讯的
  CServer(boost::asio::io_context& ioc, unsigned short& port);
  void Start();

 private:
  tcp::acceptor acceptor_;
  // 没有拷贝
  net::io_context& ioc_;
};
