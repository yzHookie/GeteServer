#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
class CServer : public std::enable_shared_from_this<CServer> {
public:
	// ioc 是与底层读写通讯的
	CServer(boost::asio::io_context& ioc, unsigned short& port);
	void Start();
private:
	tcp::acceptor acceptor_;
	net::io_context& ioc_;
	tcp::socket socket_;
};

