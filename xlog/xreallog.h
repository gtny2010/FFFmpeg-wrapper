#ifndef XREALLOG_H
#define XREALLOG_H
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "spdlog/spdlog.h"

namespace xlog {
enum class Log_Prefix { LP_TIME, LP_LEVEL, LP_LOGNAME, LP_FILENAME, LP_FUNNAME, LP_LINE, LP_THREADID, LP_PROCESSID };
enum class Log_Level { LL_TRACE, LL_DEBUG, LL_INFO, LL_WARN, LL_ERROR };

class xRealLog {
 public:
  ~xRealLog();
  explicit xRealLog(bool saveToFile = false);  // default false
  explicit xRealLog(std::string logname, bool saveToFile = false);
  xRealLog(std::string logname, int maxMsgCoun, int threadCount, bool saveToFile = false);

  xRealLog(const xRealLog &) = delete;
  xRealLog &operator=(const xRealLog &) = delete;
  xRealLog(const xRealLog &&) = delete;
  xRealLog &operator=(const xRealLog &&) = delete;

  xRealLog &setLogLevel(Log_Level logLevel);
  xRealLog &setLogPrefix(const std::vector<Log_Prefix> &prefix);
  void flush();
  std::string loggerName();

  template <typename... Args>
  void trace(const std::string &format, const char *filename_in, int line_in, const char *funcname_in,
             const Args &... args) {
    realLog(spdlog::level::trace, format, filename_in, line_in, funcname_in, args...);
  }
  template <typename... Args>
  void debug(const std::string &format, const char *filename_in, int line_in, const char *funcname_in,
             const Args &... args) {
    realLog(spdlog::level::debug, format, filename_in, line_in, funcname_in, args...);
  }
  template <typename... Args>
  void info(const std::string &format, const char *filename_in, int line_in, const char *funcname_in,
            const Args &... args) {
    realLog(spdlog::level::info, format, filename_in, line_in, funcname_in, args...);
  }
  template <typename... Args>
  void warn(const std::string &format, const char *filename_in, int line_in, const char *funcname_in,
            const Args &... args) {
    realLog(spdlog::level::warn, format, filename_in, line_in, funcname_in, args...);
  }
  template <typename... Args>
  void error(const std::string &format, const char *filename_in, int line_in, const char *funcname_in,
             const Args &... args) {
    realLog(spdlog::level::err, format, filename_in, line_in, funcname_in, args...);
  }

 protected:
  template <typename... Args>
  void realLog(spdlog::level::level_enum lvl, const std::string &format, const char *filename_in, int line_in,
               const char *funcname_in, const Args &... args) {
    m_logger->log(spdlog::source_loc(filename_in, line_in, funcname_in), lvl, format, args...);
  }
  std::string findString(Log_Prefix prefix);
  using sinkList = std::vector<spdlog::sink_ptr>;
  sinkList createSinks(const std::string &logname, bool saveToFile);
  void createSynLogger(const std::string &logname, const sinkList &sinks);
  void createAsynLogger(const std::string &logname, const sinkList &sinks);

 private:
  int m_maxMsgCount{1024 * 10};
  int m_threadWork{4};
  bool m_isSynLogger{false};
  std::vector<Log_Prefix> m_logPrefix{Log_Prefix::LP_TIME, Log_Prefix::LP_LEVEL,   Log_Prefix::LP_FILENAME,
                                      Log_Prefix::LP_LINE, Log_Prefix::LP_FUNNAME, Log_Prefix::LP_THREADID};
  const std::map<Log_Prefix, std::string> m_log_pattern_string{
      std::make_pair(Log_Prefix::LP_TIME, "[%H:%M:%S.%e]"),   std::make_pair(Log_Prefix::LP_LEVEL, "[%L]"),
      std::make_pair(Log_Prefix::LP_LOGNAME, "[%n]"),         std::make_pair(Log_Prefix::LP_FILENAME, "[%s]"),
      std::make_pair(Log_Prefix::LP_FUNNAME, "[%!]"),         std::make_pair(Log_Prefix::LP_LINE, "[%#]"),
      std::make_pair(Log_Prefix::LP_THREADID, "[thread:%t]"), std::make_pair(Log_Prefix::LP_PROCESSID, "[pid:%P]")};

  using thread_pool = spdlog::details::thread_pool;
  std::shared_ptr<thread_pool> m_asynloggertp{nullptr};
  std::shared_ptr<spdlog::logger> m_logger{nullptr};
};
}  // namespace xlog
#endif  // XREALLOG_H
