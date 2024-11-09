#include "LogicSystem.h"

#include "HttpConnection.h"

LogicSystem::LogicSystem() {
  ReqGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
    beast::ostream(connection->response_.body()) << "receive get_test req";
    int index = 0;
    for (auto& elem : connection->get_params_) {
      index++;
      beast::ostream(connection->response_.body())
          << "param " << index << "key is " << elem.first;
      beast::ostream(connection->response_.body())
          << ", param " << index << "value is " << elem.second << std::endl;
    }
  });
}
bool LogicSystem::HandleGet(const std::string& path,
                            std::shared_ptr<HttpConnection> con) {
  if (get_handlers_.find(path) == get_handlers_.end()) {
    return false;
  }
  get_handlers_[path](con);
  return true;
}
void LogicSystem::ReqGet(const std::string& url, HttpHandler handler) {
  get_handlers_.insert(std::make_pair(url, handler));
}
