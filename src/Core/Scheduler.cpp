#include "Core/Scheduler.h"
#include "Log/Log.h"
#include "Scheduler.h"

namespace {
solar::Logger::ptr g_logger = SOLAR_LOG_NAME("system");
}
namespace solar {
Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name) {
}

Scheduler::~Scheduler() {}

Fiber *Scheduler::GetMainFiber() { return nullptr; }

void Scheduler::start() {}

void Scheduler::stop() {}

void Scheduler::tickle() {}

} // namespace solar