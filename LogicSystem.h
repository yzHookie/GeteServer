#pragma once
#include "const.h"

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem : public Singleton<LogicSystem> {
  friend class Singleton<LogicSystem>;

 public:
  ~LogicSystem() = default;
  bool HandleGet(const std::string& path, std::shared_ptr<HttpConnection>);
  void ReqGet(const std::string& url, HttpHandler handler);

 private:
  LogicSystem();
  std::map<std::string, HttpHandler> post_handlers_;
  std::map<std::string, HttpHandler> get_handlers_;
};
