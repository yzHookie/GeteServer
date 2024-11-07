#pragma once

#include <iostream>
#include <memory>
#include <mutex>
template <typename T>
class Singleton {
 protected:
  Singleton() = default;
  Singleton(const Singleton<T>&) = delete;
  Singleton& operator=(const Singleton<T>& st) = delete;
  static std::shared_ptr<T> instance_;

 public:
  static std::shared_ptr<T> GetInstance() {
    static std::once_flag s_flag;
    std::call_once(s_flag, [&]() {
      // note: 这里不用 std::make_shared 的原因
      instance_ = std::shared_ptr<T>(new T);
    });
    return instance_;
  }

  void PrintAddress() {
    std::cout << "singleton address is " << instance_.get() << std::endl;
  }
  ~Singleton() { std::cout << "this is singleton destruct" << std::endl; }
};

// note: 类静态变量需要初始化，模板类就在.h文件写实现
template <typename T>
std::shared_ptr<T> Singleton<T>::instance_ = nullptr;
