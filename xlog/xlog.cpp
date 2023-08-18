#include "xlog.h"
#include "xlogmanager.h"
namespace xlog {
xRealLogPtr g_defaultLogger = std::make_shared<xRealLog>(true); // NOLINT
static std::once_flag g_logmanageFlag;
static std::shared_ptr<xLogManager> g_logManage{nullptr};

void initLogManage(const std::string &logDir, int saveDays, int packDeadline) {
  std::call_once(g_logmanageFlag, [&logDir, saveDays, packDeadline]() {
    if (!g_logManage) {
      g_logManage =
          std::make_shared<xLogManager>(logDir, saveDays, packDeadline);
    }
  });
}

xRealLogPtr createSynLog(const std::string &logname,
                         bool saveToFile /* = false*/) {
  return std::make_shared<xRealLog>(logname, saveToFile);
}
xRealLogPtr createAsynLog(const std::string &logname, int maxMsgCoun,
                          int threadCount, bool saveToFile /* = false*/) {
  return std::make_shared<xRealLog>(logname, maxMsgCoun, threadCount,
                                    saveToFile);
}
} // namespace xlog

void assert_fail(const char *__assertion, const char *__file,
                 unsigned int __line, const char *__function) {
  LOG_ERROR("{}:{}, {}, assertion: '{}'", std::string(__file), __line,
            std::string(__function), std::string(__assertion));
  LOG_ERROR("------------------------Abort!!!-------------------------------");
  // abort();
  exit(1);
}
