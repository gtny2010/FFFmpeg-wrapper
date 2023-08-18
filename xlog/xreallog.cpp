#include "xreallog.h"

#include "spdlog/async_logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "xlogutility.h"
#include "xmylog.h"
namespace xlog {
xRealLog::xRealLog(bool saveToFile) : m_isSynLogger(true) {
  std::string logname{};
  if (logname.empty()) {
    logname = xLogUtility::getAppName();
  }
  m_logger = spdlog::get(logname);
  if (!m_logger) {
    auto sinks = createSinks(logname, saveToFile);
    createSynLogger(logname, sinks);
    setLogLevel(Log_Level::LL_TRACE);
    setLogPrefix(m_logPrefix);
    m_logger->flush_on(spdlog::level::err);
    spdlog::flush_every(std::chrono::seconds(3));
  }
}
xRealLog::xRealLog(std::string logname, bool saveToFile) : m_isSynLogger(true) {
  if (logname.empty()) {
    logname = xLogUtility::getAppName();
  }
  m_logger = spdlog::get(logname);
  if (!m_logger) {
    auto sinks = createSinks(logname, saveToFile);
    createSynLogger(logname, sinks);
    setLogLevel(Log_Level::LL_TRACE);
    setLogPrefix(m_logPrefix);
    m_logger->flush_on(spdlog::level::err);
    spdlog::flush_every(std::chrono::seconds(3));
  }
}
xRealLog::xRealLog(std::string logname, int maxMsgCount, int threadCount,
                   bool saveToFile)
    : m_maxMsgCount(maxMsgCount), m_threadWork(threadCount) {
  if (logname.empty()) {
    logname = xLogUtility::getAppName();
  }
  m_logger = spdlog::get(logname);
  if (!m_logger) {
    auto sinks = createSinks(logname, saveToFile);
    createAsynLogger(logname, sinks);
    setLogLevel(Log_Level::LL_TRACE);
    setLogPrefix(m_logPrefix);
    m_logger->flush_on(spdlog::level::err);
    spdlog::flush_every(std::chrono::seconds(3));
  }
}
xRealLog::~xRealLog() {
  flush();
  spdlog::drop(m_logger->name());
}
xRealLog &xRealLog::setLogLevel(Log_Level logLevel) {
  m_logger->set_level(spdlog::level::level_enum(logLevel));
  return *this;
}

std::string xRealLog::findString(Log_Prefix prefix) {
  auto it = m_log_pattern_string.find(prefix);
  if (it != m_log_pattern_string.end()) {
    return it->second;
  }
  return "";
}
std::string xRealLog::loggerName() { return m_logger->name(); }
xRealLog &xRealLog::setLogPrefix(const std::vector<Log_Prefix> &prefix) {
  std::string pattern{};
  for (const auto &item : prefix) {
    pattern += findString(item);
  }
  pattern += " %v";
  for (const auto &sink : m_logger->sinks()) {
    sink->set_pattern(pattern);
  }
  m_logger->set_pattern(pattern);
  m_logPrefix.insert(m_logPrefix.begin(), prefix.begin(), prefix.end());
  return *this;
}
void xRealLog::flush() {
  m_logger->flush();
  if (!m_isSynLogger) {
    auto logger = std::dynamic_pointer_cast<spdlog::async_logger>(m_logger);
    m_asynloggertp->post_flush(std::move(logger),
                               spdlog::async_overflow_policy::block);
  }
}
xRealLog::sinkList xRealLog::createSinks(const std::string &logname,
                                         bool saveToFile) {
  std::vector<spdlog::sink_ptr> sink_list{};
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  sink_list.push_back(std::move(console_sink));
  if (saveToFile) {
    std::string logFile = xLogUtility::getAppDir() + "logs/";
    if (!xLogUtility::isFolderExist(logFile))
      xLogUtility::createDirectory(logFile);

    if (!xLogUtility::isFolderExist(logFile))
      xLogUtility::createDirectory(logFile);
    logFile += logname + ".txt";
    auto file_sink =
        std::make_shared<spdlog::sinks::xdaily_file_sink_mt>(logFile, 0, 0);
    sink_list.push_back(std::move(file_sink));
  }
  return sink_list;
}
void xRealLog::createSynLogger(const std::string &logname,
                               const sinkList &sinks) {
  m_logger =
      std::make_shared<spdlog::logger>(logname, sinks.begin(), sinks.end());
  if (m_logger) spdlog::register_logger(m_logger);
}
void xRealLog::createAsynLogger(const std::string &logname,
                                const sinkList &sinks) {
  m_asynloggertp = std::make_shared<thread_pool>(m_maxMsgCount, m_threadWork);
  m_logger = std::make_shared<spdlog::async_logger>(
      logname, sinks.begin(), sinks.end(), m_asynloggertp);
  if (m_logger) spdlog::register_logger(m_logger);
}
}  // namespace xlog
