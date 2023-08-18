#ifndef XLOGMANAGER_H
#define XLOGMANAGER_H
#include <atomic>
#include <thread>
namespace xlog {
class xFileManage;
class xLogManager {
public:
  explicit xLogManager(const std::string &logDir, int saveDays = 30,
                       int packDeadline = 2);
  ~xLogManager();
  xLogManager(const xLogManager &) = delete;
  xLogManager &operator=(const xLogManager &) = delete;
  xLogManager(const xLogManager &&) = delete;
  xLogManager &operator=(const xLogManager &&) = delete;
  bool isLive();

protected:
  void logStateCheck();

private:
  std::thread m_logCheckTh;
  std::atomic_bool m_isRun{false};
  std::shared_ptr<xFileManage> m_logFile{nullptr};
};
} // namespace xlog
#endif // XLOGMANAGER_H
