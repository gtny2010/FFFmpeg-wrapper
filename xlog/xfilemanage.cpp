#include "xfilemanage.h"

#include <cassert>
#include <map>
#include <thread>

#include "xlogutility.h"
namespace xlog {
xFileManage::xFileManage(std::string logDir, int saveDays, int packDeadline)
    : m_logDir(std::move(logDir)), m_logSaveDays(saveDays), m_logPackDeadline(packDeadline) {
  m_logSaveDays = (m_logSaveDays < 0) ? 0 : m_logSaveDays;
  m_logPackDeadline = (m_logPackDeadline <= 0) ? 1 : m_logPackDeadline;
}
void xFileManage::compressAndPack() {
  auto filelist = xLogUtility::findAllLogOrZipFile(m_logDir, true);
  if (filelist.empty()) {
    return;
  }

  std::map<std::string, std::vector<std::string>> zipFiles{};
  for (const auto &item : filelist) {
    auto logTime = xLogUtility::path2LogDate(item);
    if (logTime <= 0) continue;
    if (isNeedCompress(logTime)) {
      auto zipName = xLogUtility::date2string(logTime);
      if (zipName.empty()) {
        continue;
      }
      zipName += ".tar.gz";
      auto zipPrefix = item.substr(0, item.rfind('/') + 1);
      zipName = zipPrefix.append(zipName);
      zipFiles[zipName].push_back(item);
    }
  }

  for (const auto &day : zipFiles) {
    auto zipCmd = "tar --remove-files -czPf " + day.first;
    for (const auto &file : day.second) {
      zipCmd.append(" ");
      zipCmd.append(file);
    }
    if (system(zipCmd.c_str()) < 0) {
      perror("cannot compress log files");
      assert(false);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void xFileManage::removeStaledLog() {
  auto filelist = xLogUtility::findAllLogOrZipFile(m_logDir, false);
  if (filelist.empty()) {
    return;
  }

  std::vector<std::string> removeFiles{};
  for (const auto &item : filelist) {
    auto logTime = xLogUtility::path2ZipDate(item);
    if (logTime <= 0) continue;
    if (isNeedRemove(logTime)) {
      removeFiles.push_back(item);
    }
  }

  for (const auto &file : removeFiles) {
    remove(file.c_str());
  }
}

bool xFileManage::isNeedCompress(time_t oneTime) {
  auto curTime = time(nullptr);
  auto dis = curTime - oneTime;
  return (dis >= m_logPackDeadline * 24 * 60 * 60);
}
bool xFileManage::isNeedRemove(time_t oneTime) {
  auto curTime = time(nullptr);
  auto dis = curTime - oneTime;
  return (dis >= m_logSaveDays * 24 * 60 * 60);
}
}  // namespace xlog
