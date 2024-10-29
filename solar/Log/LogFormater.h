#include <memory>
#include <string>

namespace solar {
class LogFormatter {
  public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string &pattern);

  private:
    std::string m_pattern;
};
} // namespace solar