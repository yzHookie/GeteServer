#pragma once
#include "const.h"

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem : public Singleton<LogicSystem> {
  friend class Singleton<LogicSystem>;

 public:
  ~LogicSystem() = default;
  void ReqGet(const std::string& url, HttpHandler handler);
  void ReqPost(const std::string& url, HttpHandler handler);
  bool HandleGet(const std::string& path, std::shared_ptr<HttpConnection>);
  bool HandlePost(const std::string& path, std::shared_ptr<HttpConnection>);

 private:
  // 注册get，post等请求
  LogicSystem();
  std::map<std::string, HttpHandler> post_handlers_;
  std::map<std::string, HttpHandler> get_handlers_;
};
