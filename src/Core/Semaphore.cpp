#include "Semaphore.h"
#include <stdexcept>

namespace solar {
Semaphore::Semaphore(uint32_t count) {
    if (sem_init(&m_semaphore, 0, count) != 0) {
        throw std::logic_error("sem_init error");
    }
}
Semaphore::~Semaphore() { sem_destroy(&m_semaphore); }
void Semaphore::wait() {
    if (sem_wait(&m_semaphore) != 0) {
        throw std::logic_error("sem_wait error");
    }
}
void Semaphore::notify() {
    if (sem_post(&m_semaphore) != 0) {
        throw std::logic_error("sem_post error");
    }
}
} // namespace solar