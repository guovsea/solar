#include <functional>
#include <iostream>
#include <map>

#include "Log/LogFormater.h"
#include "Log/Logger.h"
#include "LogFormater.h"
namespace solar {

LogFormatter::LogFormatter(const std::string &pattern) : m_pattern(pattern) { init(); }

std::string LogFormatter::format(LogLevel level, LogEvent::ptr event) {
    std::stringstream ss;
    for (auto it: m_items) {
        it->format(ss, level, event);
    }
    return ss.str();
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, LogLevel level, LogEvent::ptr event) override { os << event->getContent(); }
};

class LogNameFormatItem : public LogFormatter::FormatItem {
public:
    LogNameFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, LogLevel level, LogEvent::ptr event) override { os << event->getLogger()->getName(); }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, LogLevel level, LogEvent::ptr event) override { os << ToString(level); }
};

class FileNameFormatItem : public LogFormatter::FormatItem {
public:
    FileNameFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, LogLevel level, LogEvent::ptr event) override { os << event->getFile(); }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, LogLevel level, LogEvent::ptr event) override { os << event->getLine(); }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, LogLevel level, LogEvent::ptr event) override { os << event->getThreadId(); }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, LogLevel level, LogEvent::ptr event) override { os << event->getFiberId(); }
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, LogLevel level, LogEvent::ptr event) override { os << "\t"; }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, LogLevel level, LogEvent::ptr event) override { os << "\n"; }
};
class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string &str) : m_string(str) {}
    void format(std::ostream &os, LogLevel level, LogEvent::ptr event) override { os << m_string; }

private:
    std::string m_string;
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, LogLevel level, LogEvent::ptr event) override { os << event->getElapse(); }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, LogLevel level, LogEvent::ptr event) override { os << event->getThreadName(); }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S") : m_format(format) {
        if (m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }

    void format(std::ostream &os, LogLevel level, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }

private:
    std::string m_format;
};

void LogFormatter::init() {
    // str, format, type
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if ((i + 1) < m_pattern.size()) {
            if (m_pattern[i + 1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while (n < m_pattern.size()) {
            if (!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}')) {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            if (fmt_status == 0) {
                if (m_pattern[n] == '{') {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    // std::cout << "*" << str << std::endl;
                    fmt_status = 1; // 解析格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if (fmt_status == 1) {
                if (m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    // std::cout << "#" << fmt << std::endl;
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if (n == m_pattern.size()) {
                if (str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
        }

        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } else if (fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }

    if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {
#define XX(str, Item)                                                                                                  \
    {                                                                                                                  \
#str, [](const std::string &fmt) { return FormatItem::ptr(new Item(fmt)); }                                    \
    }

            XX(m, MessageFormatItem), // m:消息
            XX(p, LevelFormatItem), // p:日志级别
            XX(r, ElapseFormatItem), // r:累计毫秒数
            XX(c, LogNameFormatItem), // c:日志名称
            XX(t, ThreadIdFormatItem), // t:线程 id
            XX(n, NewLineFormatItem), // n:换行
            XX(d, DateTimeFormatItem), // d:时间
            XX(f, FileNameFormatItem), // f:文件名
            XX(l, LineFormatItem), // l:行号
            XX(T, TabFormatItem), // T:Tab 键
            XX(F, FiberIdFormatItem), // F:协程 id
            XX(N, ThreadNameFormatItem) // N:线程名称
#undef XX
    };

    for (auto &i: vec) {
        if (std::get<2>(i) == 0) {
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                m_error = true;
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }

        // std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ")
        // - (" << std::get<2>(i) << ")" << std::endl;
    }
    // std::cout << m_items.size() << std::endl;
}

} // namespace solar