#ifndef __SOLAR_UTIL_SINGLETO_H__
#define __SOLAR_UTIL_SINGLETO_H__

#include <memory>

namespace solar {
/**
 * @brief
 *
 * @tparam T
 * @tparam X  传入 策略类（policy class） 或 标签类型（tag type)
 * @tparam N  区分同一类型 T 创建的多个独立的单例实例
 */
template <class T, class X = void, int N = 0> class Singleton {
public:
  static T *Instance() {
    static T v;
    return &v;
  }

  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;
  Singleton(Singleton &&) = delete;
  Singleton &operator=(Singleton &&) = delete;

protected:
  Singleton() = default;
  ~Singleton() = default;
};

template <class T, class X = void, int N = 0> class SingletonSharedPtr {
public:
  static std::shared_ptr<T> Instance() {
    std::shared_ptr<T> v(new T());
    return v;
  }

  SingletonSharedPtr(const SingletonSharedPtr &) = delete;
  SingletonSharedPtr &operator=(const SingletonSharedPtr &) = delete;
  SingletonSharedPtr(SingletonSharedPtr &&) = delete;
  SingletonSharedPtr &operator=(SingletonSharedPtr &&) = delete;

protected:
  SingletonSharedPtr() = default;
  ~SingletonSharedPtr() = default;
};
} // namespace solar

#endif /* __SOLAR_UTIL_SINGLETO_H__ */
