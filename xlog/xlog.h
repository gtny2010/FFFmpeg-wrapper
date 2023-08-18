#ifndef XLOG_H
#define XLOG_H
#include "xreallog.h"
namespace xlog {
using xRealLogPtr = std::shared_ptr<xRealLog>;
extern xRealLogPtr g_defaultLogger;
void initLogManage(const std::string &logDir, int saveDays = 30,
                   int packDeadline = 2);
// The performance of synchronous mode is better than that of asynchronous mode,
// so synchronous log is recommended
xRealLogPtr createSynLog(const std::string &logname, bool saveToFile = false);
xRealLogPtr createAsynLog(const std::string &logname, int maxMsgCoun,
                          int threadCount, bool saveToFile = false);
}  // namespace xlog

//////////////////////////////////////////////////////////////////////////////////////
[[noreturn]] void assert_fail(const char *__assertion, const char *__file,
                              unsigned int __line, const char *__function);

#define xassert(expr)      \
  (static_cast<bool>(expr) \
       ? void(0)           \
       : assert_fail(#expr, __FILE__, __LINE__, __ASSERT_FUNCTION))

//////////////////////////////////////////////////////////////////////////////////////
// using default logger handle
#define LOG_TRACE(fmt, ...) \
  XLOG_TRACE(xlog::g_defaultLogger, fmt, ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...) \
  XLOG_DEBUG(xlog::g_defaultLogger, fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) XLOG_INFO(xlog::g_defaultLogger, fmt, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) XLOG_WARN(xlog::g_defaultLogger, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
  XLOG_ERROR(xlog::g_defaultLogger, fmt, ##__VA_ARGS__)

#define LOG_ANY(lvl, fmt, ...) \
  XLOG_ANY(xlog::g_defaultLogger, lvl, fmt, ##__VA_ARGS__)

#define LOG_INTERVAL(n, lvl, fmt, ...) \
  XLOG_INTERVAL(xlog::g_defaultLogger, n, lvl, fmt, ##__VA_ARGS__)

#define LOG_TEST(condition, lvl, fmt, ...) \
  XLOG_IF(xlog::g_defaultLogger, condition, lvl, fmt, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////
// need logger handle
#define XLOG_TRACE(logger, fmt, ...)                                     \
  do {                                                                   \
    logger->trace(fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
  } while (0)

#define XLOG_DEBUG(logger, fmt, ...)                                     \
  do {                                                                   \
    logger->debug(fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
  } while (0)

#define XLOG_INFO(logger, fmt, ...)                                     \
  do {                                                                  \
    logger->info(fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
  } while (0)

#define XLOG_WARN(logger, fmt, ...)                                     \
  do {                                                                  \
    logger->warn(fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
  } while (0)

#define XLOG_ERROR(logger, fmt, ...)                                     \
  do {                                                                   \
    logger->error(fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
  } while (0)

#define XLOG_ANY(logger, lvl, fmt, ...)                                      \
  do {                                                                       \
    switch (lvl) {                                                           \
      case xlog::Log_Level::LL_TRACE:                                        \
        logger->trace(fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
        break;                                                               \
      case xlog::Log_Level::LL_DEBUG:                                        \
        logger->debug(fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
        break;                                                               \
      case xlog::Log_Level::LL_INFO:                                         \
        logger->info(fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);  \
        break;                                                               \
      case xlog::Log_Level::LL_WARN:                                         \
        logger->warn(fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);  \
        break;                                                               \
      case xlog::Log_Level::LL_ERROR:                                        \
        logger->error(fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
        break;                                                               \
    };                                                                       \
  } while (0)

#define XLOG_INTERVAL(logger, n, lvl, fmt, ...)                \
  do {                                                         \
    static int count = 0;                                      \
    if (++count > (n)) {                                       \
      count -= (n);                                            \
    }                                                          \
    if (count == 1) XLOG_ANY(logger, lvl, fmt, ##__VA_ARGS__); \
  } while (0)

#define XLOG_IF(logger, condition, lvl, fmt, ...) \
  do {                                            \
    if (condition) {                              \
      XLOG_ANY(logger, lvl, fmt, ##__VA_ARGS__);  \
    }                                             \
  } while (0)
#endif  // XLOG_H
