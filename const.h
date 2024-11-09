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