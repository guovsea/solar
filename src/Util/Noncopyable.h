#ifndef __SOLAR_UTIL_NONCOPYABLE_H__
#define __SOLAR_UTIL_NONCOPYABLE_H__
namespace solar {
class Noncopyable {
public:
  Noncopyable() = default;
  ~Noncopyable() = default;
  Noncopyable(const Noncopyable &) = delete;
  Noncopyable &operator=(const Noncopyable &) = delete;
};
} // namespace solar

#endif