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
unsigned char ToHex(unsigned char x) { return x > 9 ? x + 55 : x + 48; }

unsigned char FromHex(unsigned char x) {
  unsigned char y;
  if (x >= 'A' && x <= 'Z')
    y = x - 'A' + 10;
  else if (x >= 'a' && x <= 'z')
    y = x - 'a' + 10;
  else if (x >= '0' && x <= '9')
    y = x - '0';
  else
    assert(0);
  return y;
}

std::string UrlEncode(const std::string& str) {
  std::string strTemp = "";
  size_t length = str.length();
  for (size_t i = 0; i < length; i++) {
    // 判断是否仅有数字和字母构成
    if (isalnum((unsigned char)str[i]) || (str[i] == '-') || (str[i] == '_') ||
        (str[i] == '.') || (str[i] == '~'))
      strTemp += str[i];
    else if (str[i] == ' ')  // 为空字符
      strTemp += "+";
    else {
      // 其他字符需要提前加%并且高四位和低四位分别转为16进制（处理汉字等）
      strTemp += '%';
      strTemp += ToHex((unsigned char)str[i] >> 4);
      strTemp += ToHex((unsigned char)str[i] & 0x0F);
    }
  }
  return strTemp;
}

std::string UrlDecode(const std::string& str) {
  std::string strTemp = "";
  size_t length = str.length();
  for (size_t i = 0; i < length; i++) {
    // 还原+为空
    if (str[i] == '+') strTemp += ' ';
    // 遇到%将后面的两个字符从16进制转为char再拼接
    else if (str[i] == '%') {
      assert(i + 2 < length);
      unsigned char high = FromHex((unsigned char)str[++i]);
      unsigned char low = FromHex((unsigned char)str[++i]);
      strTemp += high * 16 + low;
    } else
      strTemp += str[i];
  }
  return strTemp;
}
void HttpConnection::HandleReq() {
  // 设置版本
  response_.version(request_.version());
  response_.keep_alive(false);
  if (request_.method() == http::verb::get) {
    // 参数解析
    PreParseGetParam();
    bool success =
        LogicSystem::GetInstance()->HandleGet(get_url_, shared_from_this());
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

  if (request_.method() == http::verb::post) {
    bool success = LogicSystem::GetInstance()->HandlePost(request_.target(),
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

void HttpConnection::PreParseGetParam() {
  // 提取 URI
  auto url = request_.target();
  // 查找查询字符串的开始位置（即 '?' 的位置）
  auto query_pos = url.find('?');
  if (query_pos == std::string::npos) {
    get_url_ = url;
    return;
  }

  get_url_ = url.substr(0, query_pos);
  std::string query_string = url.substr(query_pos + 1);
  std::string key;
  std::string value;
  size_t pos = 0;
  while ((pos = query_string.find('&')) != std::string::npos) {
    auto pair = query_string.substr(0, pos);
    size_t eq_pos = pair.find('=');
    if (eq_pos != std::string::npos) {
      key = UrlDecode(
          pair.substr(0, eq_pos));  // 假设有 url_decode 函数来处理URL解码
      value = UrlDecode(pair.substr(eq_pos + 1));
      get_params_[key] = value;
    }
    query_string.erase(0, pos + 1);
  }
  // 处理最后一个参数对（如果没有 & 分隔符）
  if (!query_string.empty()) {
    size_t eq_pos = query_string.find('=');
    if (eq_pos != std::string::npos) {
      key = UrlDecode(query_string.substr(0, eq_pos));
      value = UrlDecode(query_string.substr(eq_pos + 1));
      get_params_[key] = value;
    }
  }
}