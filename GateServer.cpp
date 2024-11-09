#include <iostream>

#include "CServer.h"

int main() {
  try {
    unsigned short port = static_cast<unsigned short>(8080);
    net::io_context ioc{1};
    // 创建中断，终止信号集
    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    // 有信号集的信号就停止
    signals.async_wait(
        [&ioc](const boost::system::error_code& error, int signal_number) {
          if (error) {
            return;
          }
          ioc.stop();
        });
    std::make_shared<CServer>(ioc, port)->Start();
    spdlog::info("Gate Server listen on port: {}", port);
    ioc.run();
  } catch (std::exception const& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return 0;
}