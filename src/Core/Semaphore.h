#ifndef __SOLAR_CORE_SEMAPHORE_H__
#define __SOLAR_CORE_SEMAPHORE_H__

#include <semaphore.h>
#include <stdint.h>

namespace solar {
class Semaphore {
  public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();
    void wait();
    void notify();

  private:
    Semaphore(const Semaphore &) = delete;
    Semaphore(const Semaphore &&) = delete;
    Semaphore &operator=(const Semaphore &) = delete;

  private:
    sem_t m_semaphore;
};
} // namespace solar

#endif /* __SOLAR_CORE_SEMAPHORE_H__ */
