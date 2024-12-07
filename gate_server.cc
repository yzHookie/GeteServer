#include <hiredis/hiredis.h>

#include <iostream>
#include "c_server.h"
#include "config_mgr.h"
#include "redis_mgr.h"
void TestRedis() {
  // 连接redis 需要启动才可以进行连接
  // redis默认监听端口为6379 可以再配置文件中修改
  redisContext* c = redisConnect("127.0.0.1", 6379);
  if (c->err) {
    printf("Connect to redisServer faile:%s\n", c->errstr);
    redisFree(c);
    return;
  }
  printf("Connect to redisServer Success\n");
  std::string redis_password = "123456";
  redisReply* r =
      (redisReply*)redisCommand(c, "AUTH %s", redis_password.c_str());
  if (r->type == REDIS_REPLY_ERROR) {
    printf("Redis认证失败: %s\n", r->str);
  } else {
    printf("Redis认证成功！\n");
  }
  // 为redis设置key
  const char* command1 = "set stest1 value1";
  // 执行redis命令行
  r = (redisReply*)redisCommand(c, command1);
  // 如果返回NULL则说明执行失败
  if (NULL == r) {
    printf("Execut command1 failure\n");
    redisFree(c);
    return;
  }
  // 如果执行失败则释放连接
  if (!(r->type == REDIS_REPLY_STATUS &&
        (strcmp(r->str, "OK") == 0 || strcmp(r->str, "ok") == 0))) {
    printf("Failed to execute command[%s]\n", command1);
    freeReplyObject(r);
    redisFree(c);
    return;
  }
  // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
  freeReplyObject(r);
  printf("Succeed to execute command[%s]\n", command1);
  const char* command2 = "strlen stest1";
  r = (redisReply*)redisCommand(c, command2);
  // 如果返回类型不是整形 则释放连接
  if (r->type != REDIS_REPLY_INTEGER) {
    printf("Failed to execute command[%s]\n", command2);
    freeReplyObject(r);
    redisFree(c);
    return;
  }
  // 获取字符串长度
  int length = r->integer;
  freeReplyObject(r);
  printf("The length of 'stest1' is %d.\n", length);
  printf("Succeed to execute command[%s]\n", command2);
  // 获取redis键值对信息
  const char* command3 = "get stest1";
  r = (redisReply*)redisCommand(c, command3);
  if (r->type != REDIS_REPLY_STRING) {
    printf("Failed to execute command[%s]\n", command3);
    freeReplyObject(r);
    redisFree(c);
    return;
  }
  printf("The value of 'stest1' is %s\n", r->str);
  freeReplyObject(r);
  printf("Succeed to execute command[%s]\n", command3);
  const char* command4 = "get stest2";
  r = (redisReply*)redisCommand(c, command4);
  if (r->type != REDIS_REPLY_NIL) {
    printf("Failed to execute command[%s]\n", command4);
    freeReplyObject(r);
    redisFree(c);
    return;
  }
  freeReplyObject(r);
  printf("Succeed to execute command[%s]\n", command4);
  // 释放连接资源
  redisFree(c);
}

void TestRedisMgr() {
  assert(RedisMgr::GetInstance()->Set("blogwebsite", "llfc.club"));
  std::string value = "";
  assert(RedisMgr::GetInstance()->Get("blogwebsite", value));
  assert(RedisMgr::GetInstance()->Get("nonekey", value) == false);
  assert(RedisMgr::GetInstance()->HSet("bloginfo", "blogwebsite", "llfc.club"));
  assert(RedisMgr::GetInstance()->HGet("bloginfo", "blogwebsite") != "");
  assert(RedisMgr::GetInstance()->ExistsKey("bloginfo"));
  assert(RedisMgr::GetInstance()->Del("bloginfo"));
  assert(RedisMgr::GetInstance()->Del("bloginfo"));
  assert(RedisMgr::GetInstance()->ExistsKey("bloginfo") == false);
  assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue1"));
  assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue2"));
  assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue3"));
  assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
  assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
  assert(RedisMgr::GetInstance()->LPop("lpushkey1", value));
  assert(RedisMgr::GetInstance()->LPop("lpushkey2", value) == false);
}

void Test() {
  TestRedisMgr();
  //TestRedis();
}
int main() {
  Test();
  ConfigMgr& g_cfg_mgr = ConfigMgr::Inst();
  std::string gate_port_str = g_cfg_mgr["GateServer"]["Port"];
  unsigned short gate_port = std::atoi(gate_port_str.c_str());
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