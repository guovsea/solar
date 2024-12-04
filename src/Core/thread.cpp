#include "Core/thread.h"
#include "Log/Log.h"

namespace solar {

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";

Thread::Thread(std::function<void()> cb, const std::string &name)
    : m_cb(cb), m_name(name) {
    if (name.empty()) {
        m_name = "UNKNOW";
    }
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if (rt) {
        SOLAR_LOG_ERROR(SOLAR_LOG_ROOT())
            << "pthread_create thread fail, rt = " << rt << "name = " << name;
    }
}
Thread::~Thread() {
    if (m_thread) {
        pthread_detach(m_thread);
    }
}
void Thread::join() {
    if (m_thread) {
        int rt = pthread_join(m_thread, nullptr);
        if (rt) {
            SOLAR_LOG_ERROR(SOLAR_LOG_ROOT())
                << "pthrad_jon thread fail, rt = " << rt
                << " name = " << m_name;
        }
        m_thread = 0;
    }
}

void* Thread::run(void* arg) {
    Thread* thread = (Thread*)arg;
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = solar::GetThreadId();
    // 修改终端中 thread 的名字
    pthread_setname_np(thread->m_thread, thread->m_name.substr(0, 15).c_str());

    std::function<void()> cb;
    cb.swap(thread->m_cb);

    cb();
    return nullptr;
}

Thread *Thread::GetThis(){
        return t_thread;
}

std::string Thread::GetName(){
    return t_thread_name;
}

void SetName(const std::string &name) {
    t_thread_name = name;
}

} // namespace solar