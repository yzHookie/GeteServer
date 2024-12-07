#pragma once
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <spdlog/spdlog.h>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <map>
#include <memory>
#include <unordered_map>

#include "singleton.h"
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

enum ErrorCodes {
  Success = 0,
  Error_Json = 1001,
  RPCFailed = 1002,
};

// Defer类
class Defer {
 public:
  // 接受一个lambda表达式或者函数指针
  Defer(std::function<void()> func) : func_(func) {}

  // 析构函数中执行传入的函数
  ~Defer() { func_(); }

 private:
  std::function<void()> func_;
};